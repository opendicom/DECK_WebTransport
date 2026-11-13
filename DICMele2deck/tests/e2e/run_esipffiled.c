// End-to-end test of ESIPFfiled.
//
// Runs a parser binary on one DICM file inside an empty temporary folder, then checks:
//   1. its exit status,
//   2. that every group file (--.  ---.  ----.  priv.) is named after the BLAKE3 of its content,
//   3. the number of frames written (files inside G/, R/ or P/).
//
// usage: run_esipffiled <binary> <file.dcm> <expected exit status> <expected frames>
// exit:  0 passed or skipped, 1 failed, 2 wrong usage or test setup error

#include <dirent.h>     // opendir, readdir, closedir
#include <limits.h>     // PATH_MAX
#include <stdio.h>
#include <stdlib.h>     // realpath, mkdtemp, system, atoi, malloc, free
#include <string.h>     // strcmp, strncmp, strrchr
#include <sys/stat.h>   // stat, S_ISREG, S_ISDIR
#include <sys/wait.h>   // WIFEXITED, WEXITSTATUS
#include <unistd.h>     // access

#include "blake3.h"

#define PASSED 0
#define FAILED 1
#define SETUP_ERROR 2

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------

// Runs a shell command. Returns its exit status (0..255),
// or -1 if it could not run or did not end normally (crash, signal).
static int run(const char *shell_command)
{
    int packed_wait_status = system(shell_command);
    if (packed_wait_status == -1 || !WIFEXITED(packed_wait_status))
        return -1;
    return WEXITSTATUS(packed_wait_status);
}

// Group files hold the attributes of one level; their name is "<prefix>.<blake3 of content>".
static int is_group_file_name(const char *file_name)
{
    return strncmp(file_name, "--.", 3) == 0      // exam
        || strncmp(file_name, "---.", 4) == 0     // series
        || strncmp(file_name, "----.", 5) == 0    // instance
        || strncmp(file_name, "priv.", 5) == 0;   // private
}

// Frames are written inside a folder named G (gray), R (RGB) or P (palette).
static int is_frame_folder(const char *folder_path)
{
    const char *last_slash = strrchr(folder_path, '/');
    const char *folder_name = last_slash ? last_slash + 1 : folder_path;
    return strcmp(folder_name, "G") == 0
        || strcmp(folder_name, "R") == 0
        || strcmp(folder_name, "P") == 0;
}

// Checks that the file name ends with the BLAKE3 of the file content, in hexadecimal.
static int check_group_hash(const char *group_file_path, const char *group_file_name, off_t group_file_size)
{
    int check_result = FAILED;
    unsigned char *file_content = malloc(group_file_size > 0 ? (size_t)group_file_size : 1);
    FILE *group_file = fopen(group_file_path, "rb");

    if (file_content == NULL || group_file == NULL) {
        fprintf(stderr, "[FAIL] cannot read %s\n", group_file_path);
        goto release;
    }
    size_t bytes_read = fread(file_content, 1, (size_t)group_file_size, group_file);
    if (bytes_read != (size_t)group_file_size) {
        fprintf(stderr, "[FAIL] short read on %s\n", group_file_path);
        goto release;
    }

    uint8_t content_hash[BLAKE3_OUT_LEN];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, file_content, (size_t)group_file_size);
    blake3_hasher_finalize(&hasher, content_hash, BLAKE3_OUT_LEN);

    char content_hash_hex[2 * BLAKE3_OUT_LEN + 1];   // 2 characters per byte + '\0'
    for (int byte_index = 0; byte_index < BLAKE3_OUT_LEN; byte_index++)
        snprintf(content_hash_hex + 2 * byte_index, 3, "%02x", content_hash[byte_index]);

    // is_group_file_name guarantees there is a '.'; the hash is what follows the last one
    const char *hash_in_file_name = strrchr(group_file_name, '.') + 1;
    if (strcmp(hash_in_file_name, content_hash_hex) != 0) {
        fprintf(stderr, "[FAIL] %s: name is not the BLAKE3 of its content (%s)\n",
                group_file_path, content_hash_hex);
        goto release;
    }
    check_result = PASSED;

release:
    if (group_file != NULL)
        fclose(group_file);
    free(file_content);
    return check_result;
}

// Walks the tree written by the parser: checks every group hash and counts the frames.
// Returns PASSED only if everything below folder_path is correct.
static int check_tree(const char *folder_path, int *frame_count)
{
    DIR *folder = opendir(folder_path);
    if (folder == NULL) {
        fprintf(stderr, "[FAIL] cannot open folder %s\n", folder_path);
        return FAILED;
    }

    int tree_result = PASSED;
    int inside_frame_folder = is_frame_folder(folder_path);
    struct dirent *folder_entry;

    while ((folder_entry = readdir(folder)) != NULL) {
        const char *entry_name = folder_entry->d_name;
        if (strcmp(entry_name, ".") == 0 || strcmp(entry_name, "..") == 0)
            continue;

        char entry_path[PATH_MAX];
        int entry_path_length = snprintf(entry_path, sizeof entry_path, "%s/%s", folder_path, entry_name);
        if (entry_path_length < 0 || entry_path_length >= (int)sizeof entry_path) {
            fprintf(stderr, "[FAIL] path too long under %s\n", folder_path);
            tree_result = FAILED;
            continue;
        }

        struct stat entry_info;
        if (stat(entry_path, &entry_info) != 0) {
            fprintf(stderr, "[FAIL] cannot stat %s\n", entry_path);
            tree_result = FAILED;
            continue;
        }

        if (S_ISDIR(entry_info.st_mode)) {
            // A failure deeper in the tree makes this level fail too; a success never undoes it.
            if (check_tree(entry_path, frame_count) != PASSED)
                tree_result = FAILED;
        } else if (S_ISREG(entry_info.st_mode)) {
            if (inside_frame_folder)
                (*frame_count)++;
            else if (is_group_file_name(entry_name)
                     && check_group_hash(entry_path, entry_name, entry_info.st_size) != PASSED)
                tree_result = FAILED;
            // anything else (log.txt, serialized.bin) is not part of the tree and is ignored
        }
    }

    closedir(folder);
    return tree_result;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "usage: %s <binary> <file.dcm> <expected exit status> <expected frames>\n", argv[0]);
        return SETUP_ERROR;
    }
    const char *binary_path_argument = argv[1];
    const char *dicm_path_argument = argv[2];
    int expected_exit_status = atoi(argv[3]);
    int expected_frame_count = atoi(argv[4]);

    // A parser target that does not build yet is skipped, not failed.
    if (access(binary_path_argument, X_OK) != 0) {
        printf("[SKIP] %s: binary not built or not executable\n", binary_path_argument);
        return PASSED;
    }
    // A missing sample is a broken test, not a skip.
    if (access(dicm_path_argument, R_OK) != 0) {
        fprintf(stderr, "[ERROR] %s: file missing or unreadable\n", dicm_path_argument);
        return SETUP_ERROR;
    }

    // Everything below is released at "cleanup", whatever happens.
    int test_result = SETUP_ERROR;
    char *binary_absolute_path = NULL;
    char *dicm_absolute_path = NULL;
    char temporary_folder[] = "/tmp/esipf_test_XXXXXX";
    int remove_temporary_folder = 0;

    // Absolute paths: the command starts with "cd" into the temporary folder,
    // after which relative paths would no longer point to the files.
    binary_absolute_path = realpath(binary_path_argument, NULL);
    dicm_absolute_path = realpath(dicm_path_argument, NULL);
    if (binary_absolute_path == NULL || dicm_absolute_path == NULL) {
        fprintf(stderr, "[ERROR] realpath failed\n");
        goto cleanup;
    }

    // The parser writes its tree in the current folder: run it in an empty one.
    if (mkdtemp(temporary_folder) == NULL) {
        fprintf(stderr, "[ERROR] cannot create a temporary folder\n");
        goto cleanup;
    }
    remove_temporary_folder = 1;

    // Two paths of up to PATH_MAX plus the fixed parts of the format always fit,
    // so the format can change without recounting. Single quotes keep paths with spaces whole.
    char parser_command[2 * PATH_MAX + 128];
    int parser_command_length = snprintf(parser_command, sizeof parser_command,
                                         "cd '%s' && '%s' '%s' > log.txt 2>&1",
                                         temporary_folder, binary_absolute_path, dicm_absolute_path);
    if (parser_command_length < 0 || parser_command_length >= (int)sizeof parser_command) {
        fprintf(stderr, "[ERROR] command does not fit in %zu bytes\n", sizeof parser_command);
        goto cleanup;
    }

    // 1. exit status
    int actual_exit_status = run(parser_command);
    if (actual_exit_status != expected_exit_status) {
        printf("[FAIL] %s: expected exit %d, got %d\n",
               dicm_path_argument, expected_exit_status, actual_exit_status);
        test_result = FAILED;
        goto keep_temporary_folder;
    }

    // 2. group hashes and 3. frames (the details of a bad hash are printed by check_tree)
    int actual_frame_count = 0;
    if (check_tree(temporary_folder, &actual_frame_count) != PASSED) {
        printf("[FAIL] %s: the written tree is not consistent\n", dicm_path_argument);
        test_result = FAILED;
        goto keep_temporary_folder;
    }
    if (actual_frame_count != expected_frame_count) {
        printf("[FAIL] %s: expected %d frame(s), got %d\n",
               dicm_path_argument, expected_frame_count, actual_frame_count);
        test_result = FAILED;
        goto keep_temporary_folder;
    }

    printf("[PASSED] %s: exit %d, %d frame(s)\n", dicm_path_argument, actual_exit_status, actual_frame_count);
    test_result = PASSED;
    goto cleanup;

keep_temporary_folder:
    // keep the parser output to inspect it
    printf("         parser output kept in %s (log.txt has its trace)\n", temporary_folder);
    remove_temporary_folder = 0;

cleanup:
    if (remove_temporary_folder) {
        char remove_command[PATH_MAX + 32];
        snprintf(remove_command, sizeof remove_command, "rm -rf '%s'", temporary_folder);
        run(remove_command);
    }
    free(dicm_absolute_path);       // free(NULL) does nothing, so this is safe on every path
    free(binary_absolute_path);
    return test_result;
}
