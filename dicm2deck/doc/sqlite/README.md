## Tutorial sqlite C

2 de febrero 2023

### [Writing a Custom SQLite Function (in C) - Part 1](https://www.openmymind.net/Writing-A-Custom-Sqlite-Function-Part-1/)

### [Writing a Custom SQLite Function (in C) - Part 2](https://www.openmymind.net/Writing-A-Custom-Sqlite-Function-Part-2/)

#### https://learning.oreilly.com/library/view/using-sqlite/

Un SQL contiene 4 modulos:

- DDL *Data Definition Language*, (CREATE IF NOT EXISTS, DROP IF EXISTS)

- DML *Data Manipulation Language*, (INSERT, UPDATE, DELETE)

- TCL *Transaction Control Language*, (BEGIN, COMMIT, ROLLBACK)

- DCL *Data Control Language* (access control)

SQLITE contiene los 3 primeros exclusivamente

#### DDL

CREATE TABLE table ( column TYPE, … )

5 data types:

- NULL,

- Integer, (−9,223,372,036,854,775,808,+9,223,372,036,854,775,807)

- Float, (8 bytes)

- Text, (UTF-8 variable length)

- BLOB (x'1234ABCD' variable length)

Lo largo max de un Text o BLOB se define en una directiva de compilación. Por defecto 1 billion bytes (casi 1 GB), opcionalmente hasta 2 GB.



affinity:

- Text affinity: NULL, Text, BLOB, Float and Int are converted to Text

- Numeric affinity: any of the 5 types, try to convert string to the appropriate numeric type

- None: no conversion



COLLATE NOCASE

DEFAULT

CURRENT_TIME

CURRENT_DATE

CURRENT_TIMESTAMP

NOT NULL

UNIQUE

CHECK ( )

PRIMARY KEY 

`ROWID`,`OID`, or `_ROWID_`



CREATE TABLE rooms
(
    room_number       INTEGER  NOT NULL,
    building_number   INTEGER  NOT NULL,
    [...,]
    PRIMARY KEY( room_number, building_number )
);



CREATE [TEMP] TABLE *`table_name`* AS SELECT *`query_statement`*;

CREATE [TEMP] VIEW *`view_name`* AS SELECT *`query_statement`*



## C API

The C API is fairly extensive, and provides full access to all of SQLite’s features. In fact, the `sqlite3` command-line utility is written using the public C API.



In all cases, if a function succeeds, it will return the constant `SQLITE_OK`, which happens to have the value zero.





`int sqlite3_open_v2( const char *filename, sqlite3 **db_ptr, int flags, const char *vfs_name )`

The `_v2` variant offers more control over how the database file is created and opened. The first two parameters are the same. The filename is assumed to be in UTF-8. There is no UTF-16 variant of this function.

A third parameter is a set of bit-field flags. These flags allow you to specify if SQLite should attempt to open the database read/write (`SQLITE_OPEN_READWRITE`), or read-only (`SQLITE_OPEN_READONLY`). If you ask for read/write access but only read-only access is available, the database will be opened in read-only mode.

The final parameter allows you to name a VFS (Virtual File System) module to use with this database connection. The VFS system acts as an abstraction layer between the SQLite library and any underlying storage system (such as a filesystem). In nearly all cases, you will want to use the default VFS module and can simply pass in a NULL pointer.

The use of the double pointer may be a bit confusing at first, but the idea behind it is simple enough. The pointer-to-a-pointer is really nothing more than a pointer that is passed by reference. This allows the function call to modify the pointer that is passed in. For example:

sqlite3    *db = NULL;
rc = sqlite3_open_v2( "database.sqlite3", &db, SQLITE_OPEN_READWRITE, NULL );
/* hopefully, db now points to a valid sqlite3 structure */

Note that `db` is an `sqlite3` pointer (`sqlite3*`), not an actual `sqlite3` structure. When we call `sqlite3_open_xxx()` and pass in the pointer reference, the open function will allocate a new `sqlite3` data structure, initialize it, and set our pointer to point to it.

This approach, including the use of a pointer reference, is a common theme in the SQLite APIs that are used to create or initialize something. They all basically work the same way and, once you get the hang of them, they are pretty straightforward and easy to use.



If the filename `:memory:` is used, then a temporary, in-memory database is created.  you can copy the contents of an in-memory database to disk (or disk to memory) using the database backup API.



#### Inicialización

```c
  sqlite3 *db;
sqlite3_initialize( );
  
  int rc = sqlite3_open_v2(
  "db.sqlite", 
  &db,
  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 
  NULL
  );

  if (rc != SQLITE_OK) { //return code
    printf("Failed to open: %s\n", sqlite3_errmsg(db));
    sqlite3_close_v2(db);
  xsqlite3_shutdown( );
    exit(1);
  }
  
  /* operations */
  
  sqlite3_close_v2(db);
  xsqlite3_shutdown( );
    exit(0);
  }
```



## Prepared Statements

The life cycle of a prepared statement is a bit complex. Unlike database connections, which are typically opened, used for some period of time, and then closed, a statement can be in a number of different states. A statement might be prepared, but not run, or it might be in the middle of processing. Once a statement has run to completion, it can be reset and re-executed multiple times before eventually being finalized and released.

/* create a statement from an SQL string */
sqlite3_stmt *stmt = NULL;
sqlite3_prepare_v2( 

sqlite3 *db, 

const char *sql_str, 

int sql_str_len, 

&stmt, 

NULL 

);
/* use the statement as many times as required */
while( ... )
{
    /* bind any parameter values */
    sqlite3_bind_xxx( stmt, param_idx, param_value... );
    ...
    /* execute statement and step over each row of the result set */
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        /* extract column values from the current result row */
        col_val = sqlite3_column_xxx( stmt, col_index );
        ...
    }
    /* reset the statement so it may be used again */
    sqlite3_reset( stmt );
    sqlite3_clear_bindings( stmt );  /* optional */
}
/* destroy and release the statement */
sqlite3_finalize( stmt );
stmt = NULL;



The first parameter is a database connection. The second parameter is an SQL command encoded in a UTF-8 or UTF-16 string. The third parameter indicates the length of the command string in bytes. The fourth parameter is a reference to a statement pointer. This is used to pass back a pointer to the new `sqlite3_stmt` structure.

The fifth parameter is a reference to a string (`char` pointer). If the command string contains multiple SQL statements and this parameter is non-NULL, the pointer will be set to the start of the next statement in the command string.

If the length parameter is negative, the length will be automatically computed by the prepare call. This requires that the command string be properly null-terminated. If the length is positive, it represents the maximum number of bytes that will be parsed. For optimal performance, provide a null-terminated string and pass a valid length value that includes the null-termination character.If the length parameter is negative, the length will be automatically computed by the prepare call. This requires that the command string be properly null-terminated. If the length is positive, it represents the maximum number of bytes that will be parsed. For optimal performance, provide a null-terminated string and pass a valid length value that includes the null-termination character.

Once a statement has been prepared, but before it is executed, you can bind parameter values to the statement. Statement parameters allow you to insert a special token into the SQL command string that represents an unspecified literal value. You can then bind specific values to the parameter tokens before the statement is executed. After execution, the statement can be reset and new parameter values can be assigned. This allows you to prepare a statement once and then re-execute it multiple times with different parameter values. This is commonly used with commands, such as `INSERT`, that have a common structure but are repeatedly executed with different values.

To execute the VDBE code, the function `sqlite3_step()` is called. This function steps through the current VDBE command sequence until some type of program break is encountered. This can happen when a new row becomes available, or when the VDBE program reaches its end, indicating that no more data is available.

In the case of a `SELECT` query, `sqlite3_step()` will return once for each row in the result set. Each subsequent call to `sqlite3_step()` will continue execution of the statement until the next row is available or the statement reaches its end.

`int sqlite3_step( sqlite3_stmt *stmt )`

Attempts to execute the provided prepared statement. If a result set row becomes available, the function will return with a value of `SQLITE_ROW`. In that case, individual column values can be extracted with the `sqlite3_column_xxx()` functions. Additional rows can be returned by making further calls to `sqlite3_step()`. If the statement execution reaches its end, the code `SQLITE_DONE` will be returned. Once this happens, `sqlite3_step()` cannot be called again with this prepared statement until the statement is first reset using `sqlite3_reset()`.`int sqlite3_step( sqlite3_stmt *stmt )`

Attempts to execute the provided prepared statement. If a result set row becomes available, the function will return with a value of `SQLITE_ROW`. In that case, individual column values can be extracted with the `sqlite3_column_xxx()` functions. Additional rows can be returned by making further calls to `sqlite3_step()`. If the statement execution reaches its end, the code `SQLITE_DONE` will be returned. Once this happens, `sqlite3_step()` cannot be called again with this prepared statement until the statement is first reset using `sqlite3_reset()`.

You can use the `sqlite3_column_xxx()` functions to inspect and extract the column values from this row. Many of these functions require a column index parameter (`cidx`). Like C arrays, the first column in a result set always has an index of zero, starting from the left.

`int sqlite3_column_count( sqlite3_stmt *stmt )`

Returns the number of columns in the statement result. If the statement does not return values, a count of zero will be returned. Valid column indexes are zero through the count minus one. (`N` columns have the indexes `0` through `N-1`).

const char* sqlite3_column_name( sqlite3_stmt *stmt, int cidx )



`const void* sqlite3_column_blob( sqlite_stmt *stmt, int cidx )`

Returns a pointer to the BLOB value from the given column. The pointer may be invalid if the BLOB has a length of zero bytes. The pointer may also be NULL if a type conversion was required.

`double sqlite3_column_double( sqlite_stmt *stmt, int cidx )`

Returns a 64-bit floating-point value from the given column.

`int sqlite3_column_int( sqlite_stmt *stmt, int cidx )`

Returns a 32-bit signed integer from the given column. The value will be truncated (without warning) if the column contains an integer value that cannot be represented in 32 bits.

`sqlite3_int64 sqlite3_column_int64( sqlite_stmt *stmt, int cidx )`

Returns a 64-bit signed integer from the given column.

`const unsigned char* sqlite3_column_text( sqlite_stmt *stmt, int cidx )`



`sqlite3_value* sqlite3_column_value( sqlite_stmt *stmt, int cidx )`

Returns a pointer to an unprotected `sqlite3_value` structure. Unprotected `sqlite3_value` structures cannot safely undergo type conversion, so you should not attempt to extract a primitive value from this structure using the `sqlite3_``value_``xxx()` functions. If you want a primitive value, you should use one of the other `sqlite3_column_xxx()` functions. The only safe use for the returned pointer is to call `sqlite3_bind_value()` or `sqlite3_result_value()`. The first is used to bind the value to another prepared statement, while the second is used to return a value in a user-defined SQL function



const char      *data = NULL;
    sqlite3_stmt    *stmt = NULL;
    /* ... open database ... */
    rc = sqlite3_prepare_v2( db, "SELECT str FROM tbl ORDER BY 1", -1, &stmt, NULL );
    if ( rc != SQLITE_OK) exit( -1 );
    while( sqlite3_step( stmt ) == SQLITE_ROW ) {
        data = (const char*)sqlite3_column_text( stmt, 0 );
        printf( "%s\n", data ? data : "[NULL]" );
    }

    sqlite3_finalize( stmt );
    /* ... close database ... */



Statement parameters are special tokens that are inserted into the SQL command string before it is passed to one of the `sqlite3_prepare_xxx()` functions. They act as a placeholder for any literal value, such as a bare number or a single quote string. After the statement is prepared, but before it is executed, you can bind specific values to each statement parameter. Once you’re done executing a statement, you can reset the statement, bind new values to the parameters, and execute the statement again—only this time with the new values.



SQLite supports five different styles of statement parameters. These short string tokens are placed directly into the SQL command string, which can then be passed to one of the `sqlite3_prepare_xxx()` functions. Once the statement is prepared, the individual parameters are referenced by index.

`?`

An anonymous parameter with automatic index. As the statement is processed, each anonymous parameter is assigned a unique, sequential index value, starting with one.

``?*`<index>`*``

Parameter with explicit numeric index. Duplicate indexes allow the same value to be bound multiple places in the same statement.

``:*`<name>`*``

A named parameter with an automatic index. Duplicate names allow the same value to be bound multiple places in the same statement.

``@*`<name>`*``

A named parameter with an automatic index. Duplicate names allow the same value to be bound multiple places in the same statement. Works exactly like the colon parameter.

``$*`<name>`*``

A named parameter with an automatic index. Duplicate names allow the same value to be bound multiple places in the same statement. This is an extended syntax to support Tcl variables. Unless you’re doing Tcl programming, I suggest you use the colon format.

To get an idea of how these work, consider this `INSERT` statement:



INSERT INTO people (id, name) VALUES ( ?, ? );



 The two statement parameters represent the `id` and `name` values being inserted. Parameter indexes start at one, so the first parameter that represents the `id` value has an index of one, and the parameter used to reference the `name` value has an index of two.

Notice that the second parameter, which is likely a text value, does not have single quotes around it. The single quotes are part of the string-literal representation, and are not required for a parameter value.



You can also use explicit index values:

INSERT INTO people (id, name) VALUES ( ?1, ?2 );



INSERT INTO people (id, name) VALUES ( :id, :name );



SELECT * FROM ?;   -- INCORRECT: Cannot use a parameter as an identifier



All the bind functions return an integer error code, which is equal to `SQLITE_OK`upon success.

The bind functions are:

`int sqlite3_bind_blob( sqlite3_stmt *stmt, int pidx, const void *data, int data_len, mem_callback )`

Binds an arbitrary length binary data BLOB.

`int sqlite3_bind_double( sqlite3_stmt *stmt, int pidx, double data )`

Binds a 64-bit floating point value.

`int sqlite3_bind_int( sqlite3_stmt *stmt, int pidx, int data )`

Binds a 32-bit signed integer value.

`int sqlite3_bind_int64( sqlite3_stmt *stmt, int pidx, sqlite3_int64 )`

Binds a 64-bit signed integer value.

`int sqlite3_bind_null( sqlite3_stmt *stmt, int pidx )`

Binds a NULL datatype.

`int sqlite3_bind_text( sqlite3_stmt *stmt, int pidx, const char *data, int data_len, mem_callback )`

Binds an arbitrary length UTF-8 encoded text value. The length is in bytes, not characters. If the length parameter is negative, SQLite will compute the length of the string up to, but not including, the null terminator. It is recommended that the manually computed lengths do not include the terminator (the terminator will be included when the value is returned).

`int sqlite3_bind_zeroblob( sqlite3_stmt *stmt, int pidx, int len )`

Binds an arbitrary length binary data BLOB, where each byte is set to zero (`0x00`). The only additional parameter is a length value, in bytes. This function is particularly useful for creating large BLOBs that can then be updated with the incremental BLOB interface. See [sqlite3_blob_open()](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/re235.html "sqlite3_blob_open()") in [Appendix G](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/apg.html "Appendix G. SQLite C API Reference") for more details.

the danger with a statement like this is that the variables need to be sanitized before they’re passed into the SQL statement. For example, consider these values:

id_val = 23;
name_val = "Fred' ); DROP TABLE people;";

This would cause our `snprintf()` to create the following SQL command sequence, with the individual commands split out onto their own lines for clarity:

INSERT INTO people( id, name ) VALUES ( 23, 'Fred' );
DROP TABLE people;
' );

While that last statement is nonsense, the second statement is cause for concern.

Thankfully, things are not quite as bad as they seem. The `sqlite3_prepare_xxx()` functions will only prepare a single statement (up to the first semicolon), unless you explicitly pass the remainder of the SQL command string to another `sqlite3_prepare_xxx()` call. That limits what can be done in a case like this, unless your code automatically prepares and executes multiple statements from a single command buffer.

Be warned, however, that the interfaces provided by many scripting languages will do exactly that, and will automatically process multiple SQL statements passed in with a single call. The SQLite convenience functions, including `sqlite3_exec()`, will also automatically process multiple SQL commands passed in through a single string. What makes `sqlite3_exec()` particularly dangerous is that the convenience functions don’t allow the use of bound values, forcing you to programmatically build SQL command statements and opening you up to problems. Later in the chapter, we’ll take a closer look at `sqlite3_exec()`and why it usually isn’t the best choice.



## scalar functions

The structure and purpose of SQL scalar functions are similar to C functions or traditional mathematical functions. The caller provides a series of function parameters and the function computes and returns a value. Sometimes these functions are purely functional (in the mathematical sense), in that they compute the result based purely off the parameters with no outside influences. In other cases, the functions are more procedural in nature, and are called to invoke specific side effects.

The body of a function can do pretty much anything you want, including calling out to other libraries. For example, you could write a function that allows SQLite to send email or query the status of a web server all through SQL functions. Your code can also interact with the database and run its own queries.

Although scalar functions can take multiple parameters, they can only return a single value, such as an integer or a string. Functions cannot return rows (a series of values), nor can they return a result set, with rows and columns.

Scalar functions can still be used to process sets of data, however. Consider this SQL statement:

SELECT format( name ) FROM employees;

In this query, the scalar function `format()` is applied to every row in the result set. This is done by calling the scalar function over and over for each row, as each row is computed. Even though the `format()` function is only referenced once in this SQL statement, when the query is executed, it can result in many different invocations of the function, allowing it to process each value from the name column.

## Registering Functions

To create a custom SQL function, you must bind an SQL function name to a C function pointer. The C function acts as a callback. Any time the SQL engine needs to invoke the named SQL function, the registered C function pointer is called. This provides a way for an SQL statement to call a C function you have written.

These functions allow you to create and bind an SQL function name to a C function pointer:

`int sqlite3_create_function( sqlite3 *db, const char *func_name, int num_param, int text_rep, void *udp, func_ptr, step_func, final_func )` `int sqlite3_create_function16( sqlite3 *db, const void *func_name, int num_param, int text_rep, void *udp, func_ptr, step_func, final_func )`

Creates a new SQL function within a database connection. The first parameter is the database connection. The second parameter is the name of the function as either a UTF-8 or UTF-16 encoded string. The third parameter is the number of expected parameters to the SQL function. If this value is negative, the number of expected parameters is variable or undefined. Fourth is the expected representation for text values passed into the function, and can be one of `SQLITE_UTF8`, `SQLITE_UTF16`, `SQLITE_UTF16BE`, `SQLITE_UTF16LE`, or `SQLITE_ANY`. This is followed by a user-data pointer.

The last three parameters are all function pointers. We will look at the specific prototypes for these function pointers later. To register and create a scalar function, only the first function pointer is used. The other two function pointers are used to register aggregate functions and should be set to NULL when defining a scalar function.

SQLite allows SQL function names to be overloaded based off both the number of parameters and the text representation. This allows multiple C functions to be associated with the same SQL function name. You can use this overloading capability to register different C implementations of the same SQL function. This might be useful to efficiently handle different text encodings, or to provide different behaviors, depending on the number of parameters.

You are not required to register multiple text encodings. When the SQLite library needs to make a function call, it will attempt to find a registered function with a matching text representation. If it cannot find an exact match, it will convert any text values and call one of the other available functions. The value `SQLITE_ANY`indicates that the function is willing to accept text values in any possible encoding.

You can update or redefine a function by simply reregistering it with a different function pointer. To delete a function, call `sqlite3_create_function_xxx()`with the same name, parameter count, and text representation, but pass in NULL for all of the function pointers. Unfortunately, there is no way to find out if a function name is registered or not, outside of keeping track yourself. That means there is no way to tell the difference between a create action and a redefine action.

It is permissible to create a new function at any time. There are limits on when you can change or delete a function, however. If the database connection has any prepared statements that are currently being executed (`sqlite3_step()` has been called at least once, but `sqlite3_reset()` has not), you cannot redefine or delete a custom function, you can only create a new one. Any attempt to redefine or delete a function will return `SQLITE_BUSY`.

If there are no statements currently being executed, you may redefine or delete a custom function, but doing so invalidates all the currently prepared statements (just as any schema change does). If the statements were prepared with `sqlite3_prepare_v2()`, they will automatically reprepare themselves next time they’re used. If they were prepared with an original version of `sqlite3_prepare()`, any use of the statement will return an `SQLITE_SCHEMA`error.

The actual C function you need to write looks like this:

`void custom_scalar_function( sqlite3_context *ctx, int num_values, sqlite3_value **values )`

This is the prototype of the C function used to implement a custom scalar SQL function. The first parameter is an `sqlite3_context` structure, which can be used to access the user-data pointer as well as set the function result. The second parameter is the number of parameter values present in the third parameter. The third parameter is an array of `sqlite3_value`pointers.

The second and third parameters (`int num_values, sqlite3_value **values`) work together in a very similar fashion to the traditional C main parameters (`int argc, char **argv`).

In a threaded application, it may be possible for different threads to call into your function at the same time. As such, user-defined functions should be thread-safe.

Most user-defined functions follow a pretty standard pattern. First, you’ll want to examine the `sqlite3_value` parameters to verify their types and extract their values. You can also extract the user-data pointer passed into `sqlite3_create_function_xxx()`. Your code can then perform whatever calculation or procedure is required. Finally, you can set the return value of the function or return an error condition.

## Extracting Parameters

SQL function parameters are passed into your C function as an array of`sqlite3_``value` structures. Each of these structures holds one parameter value.

To extract working C values from the `sqlite3_value` structures, you need to call one of the `sqlite3_value_xxx()` functions. These functions are extremely similar to the `sqlite3_column_xxx()` functions in use and design. The only major difference is that these functions take a single `sqlite3_value` pointer, rather than a prepared statement and a column index.

Like their column counterparts, the value functions will attempt to automatically convert the value into whatever datatype is requested. The conversion process and rules are the same as those used by the `sqlite3_column_xxx()` functions. See [Table 7-1](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/ch07s04.html#tbl_api_typerules "Table 7-1. SQLite type conversion rules.") for more details.

`const void* sqlite3_value_blob( sqlite3_value *value )`

Extracts and returns a pointer to a BLOB.

`double sqlite3_value_double( sqlite3_value *value )`

Extracts and returns a double-precision floating point value.

`int sqlite3_value_int( sqlite3_value *value )`

Extracts and returns a 32-bit signed integer value. The returned value will be clipped (without warning) if the parameter value contains an integer value that cannot be represented with only 32 bits.

`sqlite3_int64 sqlite3_value_int64( sqlite3_value *value )`

Extracts and returns a 64-bit signed integer value.

`const unsigned char* sqlite3_value_text( sqlite3_value *value )`

Extracts and returns a UTF-8 encoded text value. The value will always be null-terminated. Note that the returned `char` pointer is unsigned and will likely require a cast. The pointer may also be NULL if a type conversion was required.

`const void* sqlite3_value_text16( sqlite3_value *value )` `const void* sqlite3_value_text16be( sqlite3_value *value )` `const void* sqlite3_value_text16le( sqlite3_value *value )`

Extracts and returns a UTF-16 encoded string. The first function returns a string in the native byte ordering of the machine. The other two functions will return a string that is always encoded in big-endian or little-endian. The value will always be null-terminated. The pointer may also be NULL if a type conversion was required.

There are also a number of helper functions to query the native datatype of a value, as well as query the size of any returned buffers.

`int sqlite3_value_type( sqlite3_value *value )`

Returns the native datatype of the value. The return value can be one of `SQLITE_``BLOB`, `SQLITE_INTEGER`, `SQLITE_FLOAT`, `SQLITE_TEXT`, or `SQLITE_NULL`. This value can change or become invalid if a type conversion takes place.

`int sqlite3_value_numeric_type( sqlite3_value *value )`

This function attempts to convert a value into a numeric type (either `SQLITE_``FLOAT` or `SQLITE_INTEGER`). If the conversion can be done without loss of data, then the conversion is made and the datatype of the new value is returned. If a conversion cannot be done, the value will not be converted and the original datatype of the value will be returned. This can be any value that is returned by `sqlite3_value_type()`.

The main difference between this function and simply calling `sqlite3_value_double()` or `sqlite3_value_int()`, is that the conversion will only take place if it is meaningful and will not result in lost data. For example, `sqlite3_value_double()` will convert a NULL into the value 0.0, while this function will not. Similarly, `sqlite3_value_int()` will convert the first part of the string `'123xyz'` into the integer 123, ignoring the trailing `'xyz'`. This function will not, however, because no sense can be made of the trailing `'xyz'` in a numeric context.

`int sqlite3_value_bytes( sqlite3_value *value )`

Returns the number of bytes in a BLOB or in a UTF-8 encoded string. If returning the size of a text value, the size will include the null-terminator.

`int sqlite3_value_bytes16( sqlite3_value *value )`

Returns the number of bytes in a UTF-16 encoded string, including the null-terminator.

As with the `sqlite3_column_xxx()` functions, any returned pointers can become invalid if another `sqlite3_value_xxx()` call is made against the same`sqlite3_value` structure. Similarly, data conversions can take place on text datatypes when calling `sqlite3_``value_bytes()` or `sqlite3_value_bytes16()`. In general, you should follow the same rules and practices as you would with the `sqlite3_column_xxx()` functions. See [Result Columns](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/ch07s04.html#ch07_id2333144 "Result Columns") for more details.

In addition to the SQL function parameters, the `sqlite3_context` parameter also carries useful information. These functions can be used to extract either the database connection or the user-data pointer that was used to create the function.

`void* sqlite3_user_data( sqlite3_context *ctx )`

Extracts the user-data pointer that was passed into `sqlite3_create_function_``xxx()` when the function was registered. Be aware that this pointer is shared across all invocations of this function within this database connection.

`sqlite3* sqlite3_context_db_handle( sqlite3_context *ctx )`

Returns the database connection that was used to register this function.

The database connection returned by `sqlite3_context_db_handle()` can be used by the function to run queries or otherwise interact with the database.

## Returning Results and Errors

Once a function has extracted and verified its parameters, it can set about its work. When a result has been computed, that result needs to be passed back to the SQLite engine. This is done by using one of the `sqlite3_result_xxx()`functions. These functions set a result value in the function’s `sqlite3_context`structure.

Setting a result value is the only way your function can communicate back to the SQLite engine about the success or failure of the function call. The C function itself has a `void` return type, so any result or error has to be passed back through the context structure. Normally, one of the `sqlite3_result_xxx()` functions is called just prior to calling `return` within your C function, but it is permissible to set a new result multiple times throughout the function. Only the last result will be returned, however.

The `sqlite3_result_xxx()` functions are extremely similar to the `sqlite3_bind_xxx()` functions in use and design. The main difference is that these functions take an `sqlite3_context` structure, rather than a prepared statement and parameter index. A function can only return one result, so any call to an `sqlite3_result_xxx()` function will override the value set by a previous call.

`void sqlite3_result_blob( sqlite3_context* ctx, const void *data, int data_len, mem_callback )`

Encodes a data buffer as a BLOB result.

`void sqlite3_result_double( sqlite3_context *ctx, double data )`

Encodes a 64-bit floating-point value as a result.

`void sqlite3_result_int( sqlite3_context *ctx, int data )`

Encodes a 32-bit signed integer as a result.

`void sqlite3_result_int64( sqlite3_context *ctx, sqlite3_int64 data )`

Encodes a 64-bit signed integer as a result.

`void sqlite3_result_null( sqlite3_context *ctx )`

Encodes an SQL NULL as a result.

`void sqlite3_result_text( sqlite3_context *ctx, const char *data, int data_len, mem_callback )`

Encodes a UTF-8 encoded string as a result.

`void sqlite3_result_text16( sqlite3_context *ctx, const void *data, int data_len, mem_callback )` `void sqlite3_result_text16be( sqlite3_context *ctx, const void *data, int data_len, mem_callback )` `void sqlite3_result_text16le( sqlite3_context *ctx, const void *data, int data_len, mem_callback )`

Encodes a UTF-16 encoded string as a result. The first function is used for a string in the native byte format, while the last two functions are used for strings that are explicitly encoded as big-endian or little-endian, respectively.

`void sqlite3_result_zeroblob( sqlite3_context *ctx, int length )`

Encodes a BLOB as a result. The BLOB will contain the number of bytes specified, and each byte will all be set to zero (`0x00`).

`void sqlite3_result_value( sqlite3_context *ctx, sqlite3_value *result_value )`

Encodes an `sqlite3_value` as a result. A copy of the value is made, so there is no need to worry about keeping the `sqlite3_value` parameter stable between this call and when your function actually exits.

This function accepts both protected and unprotected value objects. You can pass one of the `sqlite3_value` parameters to this function if you wish to return one of the SQL function input parameters. You can also pass a value obtained from a call to `sqlite3_column_value()`.

Setting a BLOB or text value requires the same type of memory management as the equivalent `sqlite3_bind_xxx()` functions. The last parameter of these functions is a callback pointer that will properly free and release the given data buffer. You can pass a reference to `sqlite3_free()` directly (assuming the data buffers were allocated with `sqlite3_malloc()`), or you can write your own memory manager (or wrapper). You can also pass in one of the `SQLITE_TRANSIENT` or `SQLITE_STATIC` flags. See [Binding Values](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/ch07s05.html#ch07_id2304830 "Binding Values") for specifics on how these flags can be used.

In addition to encoding specific datatypes, you can also return an error status. This can be used to indicate a usage problem (such as an incorrect number of parameters) or an environment problem, such as running out of memory. Returning an error code will result in SQLite aborting the current SQL statement and returning the error back to the application via the return code of `sqlite3_step()` or one of the convenience functions.

`void sqlite3_result_error( sqlite3_context *ctx, const char *msg, int msg_size )` `void sqlite3_result_error16( sqlite3_context *ctx, const void *msg, int msg_size )`

Sets the error code to `SQLITE_ERROR` and sets the error message to the provided UTF-8 or UTF-16 encoded string. An internal copy of the string is made, so the application can free or modify the string as soon as this function returns. The last parameter indicates the size of the message in bytes. If the string is null-terminated and the last parameter is negative, the string size is automatically computed.

`void sqlite3_result_error_toobig( sqlite3_context *ctx )`

Indicates the function could not process a text or BLOB value due to its size.

`void sqlite3_result_error_nomem( sqlite3_context *ctx )`

Indicates the function could not complete because it was unable to allocate required memory. This specialized function is designed to operate without allocating any additional memory. If you encounter a memory allocation error, simply call this function and have your function return.

`void sqlite3_result_error_code( sqlite3_context *ctx, int code )`

Sets a specific SQLite error code. Does not set or modify the error message.

It is possible to return both a custom error message and a specific error code. First, call `sqlite3_result_error()` (or `sqlite3_result_error16()`) to set the error message. This will also set the error code to `SQLITE_ERROR`. If you want a different error code, you can call `sqlite3_result_error_code()` to override the generic error code with something more specific, leaving the error message untouched. Just be aware that `sqlite3_``result_error()` will always set the error code to `SQLITE_ERROR`, so you must set the error message before you set a specific error code.

## Example

Here is a simple example that exposes the SQLite C API function `sqlite3_limit()` to the SQL environment as the SQL function `sql_limit()`. This function is used to adjust various limits associated with the database connection, such as the maximum number of columns in a result set or the maximum size of a BLOB value.

Here’s a quick introduction to the C function `sqlite3_limit()`, which can be used to adjust the soft limits of the SQLite environment:

`int sqlite3_limit( sqlite3 *db, int limit_type, int limit_value )`

For the given database connection, this sets the limit referenced by the second parameter to the value provided in the third parameter. The old limit is returned. If the new value is negative, the limit value will remain unchanged. This can be used to probe an existing limit. The soft limit cannot be raised above the hard limit, which is set at compile time.

For more specific details on `sqlite3_limit()`, see [sqlite3_limit()](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/re287.html "sqlite3_limit()") in [Appendix G](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/apg.html "Appendix G. SQLite C API Reference"). You don’t need a full understanding of how this API call works to understand these examples.

Although the `sqlite3_limit()` function makes a good example, it might not be the kind of thing you’d want to expose to the SQL language in a real-world application. In practice, exposing this C API call to the SQL level brings up some security concerns. Anyone that can issue arbitrary SQL calls would have the capability of altering the SQLite soft limits. This could be used for some types of denial-of-service attacks by raising or lowering the limits to their extremes.

### sql_set_limit

In order to call the `sqlite3_limit()` function, we need to determine the `limit_type` and `value` parameters. This will require an SQL function that takes two parameters. The first parameter will be the limit type, expressed as a text constant. The second parameter will be the new limit. The SQL function can be called like this to set a new expression-depth limit:

SELECT sql_limit( 'EXPR_DEPTH', 400 );

The C function that implements the SQL function `sql_limit()` has four main parts. The first task is to verify that the first SQL function parameter (passed in as `values[0]`) is a text value. If it is, the function extracts the text to the `str` pointer:

static void sql_set_limit( sqlite3_context *ctx, int
                                                 num_values, sqlite3_value **values )
{
    sqlite3      *db = sqlite3_context_db_handle( ctx );
    const char   *str = NULL;
    int           limit = -1, val = -1, result = -1;
    /* verify the first param is a string and extract pointer */
    if ( sqlite3_value_type( values[0] ) == SQLITE_TEXT ) {
        str = (const char*) sqlite3_value_text( values[0] );
    } else {
        sqlite3_result_error( ctx, "sql_limit(): wrong parameter type", -1 );
        return;
    }

Next, the function verifies that the second SQL parameter (`values[1]`) is an integer value, and extracts it into the `val` variable:

    /* verify the second parameter is an integer and extract value */
    if ( sqlite3_value_type( values[1] ) == SQLITE_INTEGER ) {
        val = sqlite3_value_int( values[1] );
    } else {
        sqlite3_result_error( ctx, "sql_limit(): wrong parameter type", -1 );
        return;
    }

Although our SQL function uses a text value to indicate which limit we would like to modify, the C function `sqlite3_limit()` requires a predefined integer value. We need to decode the `str` text value into an integer `limit` value. I’ll show the code to `decode_limit_str()` in just a bit:

    /* translate string into integer limit */
    limit = decode_limit_str( str );
    if ( limit == -1 ) {
        sqlite3_result_error( ctx, "sql_limit(): unknown limit type", -1 );
        return;
    }

After verifying our two SQL function parameters, extracting their values, and translating the text limit indicator into a proper integer value, we finally call `sqlite3_``limit()`. The result is set as the result value of the SQL function and the function returns:

    /* call sqlite3_limit(), return result */
    result = sqlite3_limit( db, limit, val );
    sqlite3_result_int( ctx, result );
    return;

}

The `decode_limit_str()` function is very simple, and simply looks for a predefined set of text values:

int decode_limit_str( const char *str )
{
    if ( str == NULL ) return -1;    
    if ( !strcmp( str, "LENGTH"          ) ) return SQLITE_LIMIT_LENGTH;
    if ( !strcmp( str, "SQL_LENGTH"      ) ) return SQLITE_LIMIT_SQL_LENGTH;
    if ( !strcmp( str, "COLUMN"          ) ) return SQLITE_LIMIT_COLUMN;
    if ( !strcmp( str, "EXPR_DEPTH"      ) ) return SQLITE_LIMIT_EXPR_DEPTH;
    if ( !strcmp( str, "COMPOUND_SELECT" ) ) return SQLITE_LIMIT_COMPOUND_SELECT;
    if ( !strcmp( str, "VDBE_OP"         ) ) return SQLITE_LIMIT_VDBE_OP;
    if ( !strcmp( str, "FUNCTION_ARG"    ) ) return SQLITE_LIMIT_FUNCTION_ARG;
    if ( !strcmp( str, "ATTACHED"        ) ) return SQLITE_LIMIT_ATTACHED;
    if ( !strcmp( str, "LIKE_LENGTH"     ) ) return SQLITE_LIMIT_LIKE_PATTERN_LENGTH;
    if ( !strcmp( str, "VARIABLE_NUMBER" ) ) return SQLITE_LIMIT_VARIABLE_NUMBER;
    if ( !strcmp( str, "TRIGGER_DEPTH"   ) ) return SQLITE_LIMIT_TRIGGER_DEPTH;
    return -1;
}

With these two functions in place, we can create the `sql_limit()` SQL function by registering the `sql_set_limit()` C function pointer.

sqlite3_create_function( db, "sql_limit", 2, SQLITE_UTF8, 
                         NULL, sql_set_limit, NULL, NULL );

The parameters for this function include the database connection (`db`), the name of the SQL function (`sql_limit`), the required number of parameters (`2`), the expected text encoding (UTF-8), the user-data pointer (NULL), and finally the C function pointer that implements this function (`sql_set_limit`). The last two parameters are only used when creating aggregate functions, and are set to NULL.

Once the SQL function has been created, we can now manipulate the limits of our SQLite environment by issuing SQL commands. Here are some examples of what the `sql_limit()` SQL function might look like if we integrated it into the `sqlite3` tool (we’ll see how to do this using a loadable extension later in the chapter).

First, we can look up the current `COLUMN` limit by passing a new limit value of `-1`:

sqlite> **`SELECT sql_limit( 'COLUMN', -1 );`**
2000

We verify the function works correctly by setting the maximum column limit to two, and then generating a result with three columns. The previous limit value is returned when we set the new value:

sqlite> **`SELECT sql_limit( 'COLUMN', 2 );`**
2000
sqlite> **`SELECT 1, 2, 3;`**
Error: too many columns in result set

We see from the error that the soft limit is correctly set, meaning our function is working.

One thing you might be wondering about is parameter value count. Although the `sql_set_limit()` function carefully checks the types of the parameters, it doesn’t actually verify that `num_values` is equal to two. In this case, it doesn’t have to, since it was registered with `sqlite3_create_function()` with a required parameter count of two. SQLite will not even call our `sql_set_limit()`function unless we have exactly two parameters:

sqlite> **`SELECT sql_limit( 'COLUMN', 2000, 'extra' );`**
Error: wrong number of arguments to function sql_limit()

SQLite sees the wrong number of parameters and generates an error for us. This means that as long as a function is registered correctly, SQLite will do some of our value checking for us.

### sql_get_limit

While having a fixed parameter count simplifies the verification code, it might be useful to provide a single-parameter version that can be used to look up the current value. This can be done a few different ways. First, we can define a second C function called `sql_get_limit()`. This function would be the same as `sql_set_limit()`, but with the second block of code removed:

    /* remove this block of code from a copy of   */
    /* sql_set_limit() to produce sql_get_limit() */
    if ( sqlite3_value_type( values[1] ) == SQLITE_INTEGER ) {
        val = sqlite3_value_int( values[1] );
    } else {
        sqlite3_result_error( ctx, "sql_limit(): wrong parameter type", -1 );
        return;
    }

With this code removed, the function will never decode the second SQL function parameter. Since `val` is initialized to –1, this effectively makes every call a query call. We register each of these functions separately:

sqlite3_create_function( db, "sql_limit", 1,
        SQLITE_UTF8, NULL, sql_get_limit, NULL, NULL );
sqlite3_create_function( db, "sql_limit", 2,
        SQLITE_UTF8, NULL, sql_set_limit, NULL, NULL );

This dual registration overloads the SQL function name `sql_limit()`. Overloading is allowed because the two calls to `sqlite3_create_function()` have a different number of required parameters. If the SQL function `sql_limit()` is called with one parameter, then the C function `sql_get_limit()` is called. If two parameters are provided to the SQL function, then the C function `sql_set_limit()` is called.

### sql_getset_limit

Although the two C functions `sql_get_limit()` and `sql_set_limit()` provide the correct functionality, the majority of their code is the same. Rather than having two functions, it might be simpler to combine these two functions into one function that can deal with either one or two parameters, and is capable of both getting or setting a limit value.

This combine `sql_getset_limit()` function can be created by taking the original `sql_set_limit()` function and modifying the second section. Rather than eliminating it, as we did to create `sql_get_limit()`, we’ll simply wrap it in an `if` statement, so the second section (which extracts the second SQL function parameter) is only run if we have two parameters:

    /* verify the second parameter is an integer and extract value */
    if ( num_values == 2 ) {
        if ( sqlite3_value_type( values[1] ) == SQLITE_INTEGER ) {
            val = sqlite3_value_int( values[1] );
        } else {
            sqlite3_result_error( ctx, "sql_limit(): wrong parameter type", -1 );
            return;
        }
    }

We register the same `sql_getset_limit()` C function under both parameter counts:

sqlite3_create_function( db, "sql_limit", 1,
        SQLITE_UTF8, NULL, sql_getset_limit, NULL, NULL );
sqlite3_create_function( db, "sql_limit", 2,
        SQLITE_UTF8, NULL, sql_getset_limit, NULL, NULL );

For this specific task, this is likely the best choice. SQLite will verify the SQL function `sql_limit()` has exactly one or two parameters before calling our C function, which can easily deal with either one of those two cases.

### sql_getset_var_limit

If for some reason you don’t like the idea of registering the same function twice, we could also have SQLite ignore the parameter count and call our function no matter what. This leaves verification of a valid parameter count up to us. To do that, we’d start with the `sql_getset_limit()` function and change it to `sql_getset_var_limit()`, by adding this block at the top of the function:

    if ( ( num_values < 1 )||( num_values > 2 ) ) {
        sqlite3_result_error( ctx, "sql_limit(): bad parameter count", -1 );
        return;
    }

We register just one version. By passing a required parameter count of `-1`, we’re telling the SQLite engine that we’re willing to accept any number of parameters:

sqlite3_create_function( db, "sql_limit", -1, SQLITE_UTF8,
        NULL, sql_getset_var_limit, NULL, NULL );

Although this works, the `sql_getset_limit()` version is still my preferred version. The registration makes it clear which versions of the function are considered valid, and the function code is reasonably clear and compact.

Completely free-form parameter counts are usually used by items like the built-infunction `coalesce()`. The `coalesce()` function will take any number of parameters (greater than one) and return the first non-NULL value in the list. Since you might pass anywhere from two to a dozen or more parameters, it is impractical to register each possible configuration, and is better to just allow the function to do its own parameter management.

On the other hand, something like `sql_getset_limit()` can really only accept two configurations: one parameter or two. In that case, I find it easier to explicitly register both parameter counts and allow SQLite to do my parameter verificationfor me.





# Aggregate Functions

Aggregate functions are used to collapse values from a grouping of rows into a single result value. This can be done with a whole table, as is common with the aggregate function `count(*)`, or it can be done with groupings of rows from a `GROUP BY` clause, as is common with something like `avg()` or `sum()`. Aggregate functions are used to summarize, or aggregate, all of the individual row values into some single representative value.

## Defining Aggregates

SQL aggregate functions are created using the same `sqlite3_create_function_xxx()` function that is used to create scalar functions (See [Scalar Functions](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/ch09s01.html "Scalar Functions")). When defining a scalar function, you pass in a C function pointer in the sixth parameter and set the seventh and eighth parameter to NULL. When defining an aggregate function, the sixth parameter is set to NULL (the scalar function pointer) and the seventh and eighth parameters are used to pass in two C function pointers.

The first C function is a “step” function. It is called once for each row in an aggregate group. It acts similarly to an scalar function, except that it does not return a result (it may return an error, however).

The second C function is a “finalize” function. Once all the SQL rows have been stepped over, the finalize function is called to compute and set the final result. The finalize function doesn’t take any SQL parameters, but it is responsible for setting the result value.

The two C functions work together to implement the SQL aggregate function. Consider the built-in `avg()` aggregate, which computes the numeric average of all the rows in a column. Each call to the step function extracts an SQL value for that row and updates both a running total and a row count. The finalize function divides the total by the row count and sets the result value of the aggregate function.

The C functions used to implement an aggregate are defined like this:

`void user_aggregate_step( sqlite3_context *ctx, int num_values, sqlite3_value **values )`

The prototype of a user-defined aggregate step function. This function is called once for each row of an aggregate calculation. The prototype is the same as a scalar function and all of the parameters have similar meaning. The step function should not set a result value with `sqlite3_result_xxx()`, but it may set an error.

`void user_aggregate_finalize( sqlite3_context *ctx )`

The prototype of a user-defined aggregate finalize function. This function is called once, at the end of an aggregation, to make the final calculation and set the result. This function should set a result value or error condition.

Most of the rules about SQL function overloading that apply to scalar functions also apply to aggregate functions. More than one set of C functions can be registered under the same SQL function name if different parameter counts or text encodings are used. This is less commonly used with aggregates, however, as most aggregate functions are numeric-based and the majority of aggregates take only one parameter.

It is also possible to register both scalar and aggregate functions under the same name, as long as the parameter counts are different. For example, the built-in `min()` and `max()` SQL functions are available as both scalar functions (with two parameters) and aggregate functions (with one parameter).

Step and finalize functions can be mixed and matched—they don’t always need to be unique pairs. For example, the built-in `sum()` and `avg()` aggregates both use the same step function, since both aggregates need to compute a running total. The only difference between these aggregates is the finalize function. The finalize function for `sum()` simply returns the grand total, while the finalize function for`avg()` first divides the total by the row count.

## Aggregate Context

Aggregate functions typically need to carry around a lot of state. For example, the built-in `avg()` aggregate needs to keep track of the running total, as well as the number of rows processed. Each call to the step function, as well as the finalize function, needs access to some shared block of memory that holds all the state values.

Although aggregate functions can call `sqlite3_user_data()` or `sqlite3_context_db_handle()`, you can’t use the user-data pointer to store aggregate state data. The user-data pointer is shared by all instances of a given aggregate function. If more than one instance of the aggregate function is active at the same time (for example, an SQL query that averages more than one column), each instance of the aggregate needs a private copy of the aggregate state data, or the different aggregate calculations will get intermixed.

Thankfully, there is an easy solution. Because almost every aggregate function requires some kind of state data, SQLite allows you to attach a data-block to each specific aggregate instance.

`void* sqlite3_aggregate_context( sqlite3_context *ctx, int bytes )`

This function can be called inside an aggregate step function or finalize function. The first parameter is the `sqlite3_context` structure passed into the step or finalize function. The second parameter represents a number of bytes.

The first time this function is called within a specific aggregate instance, the function will allocate an appropriately sized block of memory, zero it out, and attach it to the aggregate context before returning a pointer. This function will return the same block of memory in subsequent invocations of the step and finalize functions. The memory block is automatically deallocated when the aggregate goes out of scope.

Using this API call, you can have the SQLite engine automatically allocate and release your aggregate state data on a per-instance basis. This allows multiple instances of your aggregate function to be active simultaneously without any extra work on your part.

Typically, one of the first things a step or finalize function will do is call `sqlite3_aggregate_context()`. For example, consider this oversimplified version of sum:

void simple_sum_step( sqlite3_context *ctx, int num_values, sqlite3_value **values )
{
    double *total = (double*)sqlite3_aggregate_context( ctx, sizeof( double ) );
    *total += sqlite3_value_double( values[0] );
}
void simple_sum_final( sqlite3_context *ctx )
{
    double *total = (double*)sqlite3_aggregate_context( ctx, sizeof( double ) );
    sqlite3_result_double( ctx, *total );
}
/* ...inside an initialization function... */
    sqlite3_create_function( db, "simple_sum", 1, SQLITE_UTF8, NULL, 
            NULL, simple_sum_step, simple_sum_final );

In this case, we’re only allocating enough memory to hold a double-precision floating-point value. Most aggregate functions will allocate a C struct with whatever fields are required to compute the aggregate, but everything works the same way. The first time `simple_sum_step()` is called, the call to `sqlite3_aggregate_context()` will allocate enough memory to hold a double and zero it out. Subsequent calls to `simple_sum_``step()` that are part of the same aggregation calculation (have the same `sqlite3_context`) will have the same block of memory returned, as will `simple_sum_final()`.

Because `sqlite3_aggregate_context()` may need to allocate memory, it is also a good idea to make sure the returned value is not NULL. The above code, in both the step and finalize functions, should really look something like this:

    double *total = (double*)sqlite3_aggregate_context( ctx, sizeof( double ) );
    if ( total == NULL ) {
        sqlite3_result_error_nomem( ctx );
        return;
    }

The only caution with `sqlite3_aggregate_context()` is in properly dealing with data structure initialization. Because the context data structure is silently allocated and zeroed out on the first call, there is no obvious way to tell the difference between a newly allocated structure, and one that was allocated in a previous call to your step function.

If the default all-zero state of a newly allocated context is not appropriate, and you need to somehow initialize the aggregate context, you’ll need to include some type of initialization flag. For example:

typedef struct agg_state_s {
    int    init_flag;
    /* other fields used by aggregate... */
} agg_state;

The aggregate functions can use this flag to determine if it needs to initialize the aggregate context data or not:

    agg_state *st = (agg_state*)sqlite3_aggregate_context( ctx, sizeof( agg_state ) );
    /* ...return nonmem error if st == NULL... */
    if ( st->init_flag == 0 ) {
        st->init_flag = 1;
        /* ...initialize the rest of agg_state... */
    }

Since the structure is zeroed out when it is first allocated, your initialization flag will be zero on the very first call. As long as you set the flag to something else when you initialize the rest of the data structure, you’ll always know if you’re dealing with a new allocation that needs to be initialized or an existing allocation that has already been initialized.

Be sure to check the initialization flag in both the step function and the finalize function. There are cases when the finalize function may be called without first calling the step function, and the finalize function needs to properly deal with those cases.

## Example

As a more in-depth example, let’s look at a *weighted average* aggregate. Although most aggregates take only one parameter, our `wtavg()` aggregate will take two. The first parameter will be whatever numeric value we’re trying to average, while the second, optional parameter will be a weighting for this row. If a row has a weight of two, its value will be considered to be twice as important as a row with a weighting of only one. A weighted average is taken by summing the product of the values and weights, and dividing by the sum of the weights.

To put things in SQL terms, if our `wtavg()` function is used like this:

SELECT wtavg( data, weight ) FROM ...

It should produce results that are similar to this:

SELECT ( sum( data * weight ) / sum( weight ) ) FROM ...

The main difference is that our `wtavg()` function should be a bit more intelligent about handling invalid weight values (such as a NULL) and assign them a weight value of 1.0.

To keep track of the total data values and the total weight values, we need to define an aggregate context data structure. This will hold the state data for our aggregate. The only place this structure is referenced is the two aggregate functions, so there is no need to put it in a separate header file. It can be defined in the code right along with the two functions:

typedef struct wt_avg_state_s {
   double   total_data;  /* sum of (data * weight) values */
   double   total_wt;    /* sum of weight values */
} wt_avg_state;

Since the default initialization state of zero is exactly what we want, we don’t need a separate initialization flag within the data structure.

In this example, I’ve made the second aggregate function parameter (the weight value) optional. If only one parameter is provided, all the weights are assumed to be one, resulting in a traditional average. This will still be different than the built-in `avg()` function, however. SQLite’s built-in `avg()` function follows the SQL standard in regard to typing and NULL handling, which might not be what you first assume. (See [avg()](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/re160.html "avg()") in [Appendix E](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/ape.html "Appendix E. SQLite SQL Function Reference") for more details). Our `wtavg()` is a bit simpler. In addition to always returning a double (even if the result could be expressed as an integer), it simply ignores any values that can’t easily be translated into a number.

First, the step function. This processes each row, adding up the value-weight products, as well as the total weight value:

void wt_avg_step( sqlite3_context *ctx, int num_values, sqlite3_value **values )
{
    double         row_wt = 1.0;
    int            type;
    wt_avg_state   *st = (wt_avg_state*)sqlite3_aggregate_context( ctx,
                                               sizeof( wt_avg_state ) );
    if ( st == NULL ) {
        sqlite3_result_error_nomem( ctx );
        return;
    }
    /* Extract weight, if we have a weight and it looks like a number */
    if ( num_values == 2 ) {
        type = sqlite3_value_numeric_type( values[1] );
        if ( ( type == SQLITE_FLOAT )||( type == SQLITE_INTEGER ) ) {
            row_wt = sqlite3_value_double( values[1] );
        }
    }
    /* Extract data, if we were given something that looks like a number. */
    type = sqlite3_value_numeric_type( values[0] );
    if ( ( type == SQLITE_FLOAT )||( type == SQLITE_INTEGER ) ) {
        st->total_data += row_wt * sqlite3_value_double( values[0] );
        st->total_wt   += row_wt;
    }
}

Our step function uses `sqlite3_value_numeric_type()` to try to convert the parameter values into a numeric type without loss. If the conversion is possible, we always convert the values to a double-precision floating-point, just to keep things simple. This approach means the function will work properly with text representations of numbers (such as the string `'153'`), but will ignore other datatypes and other strings.

In this case, the function does not report an error, it just ignores the value. If the weight cannot be converted, it is assumed to be one. If the data value cannot be converted, the row is skipped.

Once we have our totals, we need to compute the final answer and return the result. This is done in the finalize function, which is pretty simple. The main thing we need to worry about is the possibility of dividing by zero:

void wt_avg_final( sqlite3_context *ctx )
{
    double         result = 0.0;
    wt_avg_state   *st = (wt_avg_state*)sqlite3_aggregate_context( ctx,
                                               sizeof( wt_avg_state ) );
    if ( st == NULL ) {
        sqlite3_result_error_nomem( ctx );
        return;
    }
    if ( st->total_wt != 0.0 ) {
        result = st->total_data / st->total_wt;
    }
    sqlite3_result_double( ctx, result );
}

To use our aggregate, our application code needs to register these two functions with a database connection using `sqlite3_create_function()`. Since the `wtavg()` aggregate is designed to take either one or two parameters, we’ll register it twice:

    sqlite3_create_function( db, "wtavg", 1, SQLITE_UTF8, NULL,
            NULL, wt_avg_step, wt_avg_final );
    sqlite3_create_function( db, "wtavg", 2, SQLITE_UTF8, NULL,
            NULL, wt_avg_step, wt_avg_final );

Here are some example queries, as seen from the `sqlite3` command shell. This assumes we’ve integrated our custom aggregate into the `sqlite3` code (an example of the different ways to do this is given later in the chapter):

sqlite> **`SELECT class, value, weight FROM t;`**

class       value       weight

----------  ----------  ----------

1           3.4         1.0
1           6.4         2.3
1           4.3         0.9
2           3.4         1.4
3           2.7         1.1
3           2.5         1.1

First, we can try things with only one parameter. This will use the default 1.0 weight for each row, resulting in a traditional average calculation:

sqlite> **`SELECT class, wtavg( value ) AS wtavg, avg( value ) AS avg`**
   ...>   **`FROM t GROUP BY 1;`**

class       wtavg       avg       

----------  ----------  ----------

1           4.7         4.7       
2           3.4         3.4       
3           2.6         2.6       

And finally, here is an example of the full weighted-average calculation:

sqlite> **`SELECT class, wtavg( value, weight ) AS wtavg, avg( value ) AS avg`**
   ...>   **`FROM t GROUP BY 1;`**

class       wtavg             avg       

----------  ----------------  ----------

1           5.23571428571428  4.7       
2           3.4               3.4       
3           2.6               2.6       

In the case of `class=1`, we see a clear difference, where the heavily weighted 6.4 draws the average higher. For `class=2`, there is only one value, so the weighted and unweighted averages are the same (the value itself). In the case of `class=3`, the weights are the same for all values, so again, the average is the same as an unweighted average.





# Collation Functions

Collations are used to sort text values. They can be used with `ORDER BY` or `GROUP BY` clauses, or for defining indexes. You can also assign a collation to a table column, so that any index or ordering operation applied to that column will automatically use a specific collation. Above everything else, SQLite will always sort by datatype. NULLs will always come first, followed by a mix of integer and floating-point numeric values in their natural sort order. After the numbers come text values, followed by BLOBs.

Most types have a clearly defined sort order. NULL types have no values, so they cannot be sorted. Numeric types use their natural numeric ordering, and BLOBs are always sorted using binary comparisons. Where things get interesting is when it comes to text values.

The default collation is known as the `BINARY` collation. The `BINARY` collation sorts individual bytes using a simple numeric comparison of the underlying character encoding. The `BINARY` collation is also used for BLOBs.

In addition to the default `BINARY` collation, SQLite includes a built-in `NOCASE`and `RTRIM` collation that can be used with text values. The `NOCASE` collation ignores character case for the purposes of sorting 7-bit ASCII, and would consider the expression `'A' == 'a'` to be true. It does not, however, consider `'Ä' == 'ä'` to be true, nor does it consider `'Ä' == 'A'` to be true, as the representations of these characters are outside of the ASCII standard. The `RTRIM` collation (right-trim) is similar to the default `BINARY` collation, only it ignores trailing whitespace (that is, whitespace on the right side of the value).

While these built-in collations offer some basic options, there are times when complex sort ordering is required. This is especially true when you get into Unicode representations of languages that cannot be represented with a simple 7-bit ASCII encoding. You may also need a specialized sorting function that sorts by whole words or groups of characters if you’re storing something other than natural language text. For example, if you were storing gene sequences as text data, you might require a custom sorting function for that data.

User-defined collation functions allow the developer to define a new collation by registering a comparison function. Once registered, this function is used to compare strings as part of any sorting process. By defining the basic comparison operator, you essentially define the behavior of the whole collation.

## Registering a Collation

To define a custom collation, an application needs to register a comparison function under a collation name. Anytime the database engine needs to sort something under that collation, it uses the comparison function to define the required ordering. You will need to reregister the collation with each database connection that requires it.

There are three API calls that can be used to register a collation comparison function:

`int sqlite3_create_collation( sqlite3 *db, const char *name, int text_rep, void *udp, comp_func )` `int sqlite3_create_collation16( sqlite3 *db, const void *name, int text_rep, void *udp, comp_func )`

Registers a collation comparison function with a database connection. The first parameter is the database connection. The second parameter is the name of the custom collation encoded as a UTF-8 or UTF-16 string. The third parameter is the string encoding the comparison function expects, and can be one of `SQLITE_UTF8`, `SQLITE_UTF16`, `SQLITE_UTF16BE`, `SQLITE_UTF16LE`, or `SQLITE_UTF16_ALIGNED` (native UTF-16 that is 16-bit memory aligned). The fourth parameter is a generic user-data pointer that is passed to your comparison function. The last parameter is a function pointer to your comparison function (the prototype of this function is given below).

You can unregister a collation by passing a NULL function pointer in under the same name and text encoding as it was originally registered.

`int sqlite3_create_collation_v2( sqlite3 *db, const char *name, int text_rep, void *udp, comp_func, dest_func )`

This function is the same as `sqlite3_create_collation()`, with one additional parameter. The additional sixth parameter is an optional function pointer referencing a clean-up function that is called when the collation is destroyed (the prototype of this function is given below). This allows the collation to release any resources associated with the collation (such as the user-data pointer). A NULL function pointer can be passed in if no destroy function is required.

A collation is destroyed when the database connection is closed, when a replacement collation is registered, or when the collation name is cleared by binding a NULL comparison function pointer.

The collation name is case-insensitive. SQLite allows multiple C sorting functions to be registered under the same name, so long as they take different text representations. If more than one comparison function is available under the same name, SQLite will pick the one that requires the least amount of conversion. If you do register more than one function under the same name, their logical sorting behavior should be the same.

The format of the user-defined function pointers is given below.

`int user_defined_collation_compare( void* udp, int lenA, const void *strA, int lenB, const void *strB )`

This is the function type of a user-defined collation comparison function. The first parameter is the user-data pointer passed into `sqlite3_create_collation_xxx()` as the fourth parameter. The parameters that follow pass in the length and buffer pointers for two strings. The strings will be in whatever encoding was defined by the register function. You cannot assume the strings are null-terminated.

The return value should be negative if string A is less than string B (that is, A sorts before B), 0 if the strings are considered equal, and positive if string A is greater than B (A sorts after B). In essence, the return value is the ordering of A minus B.

`void user_defined_collation_destroy( void *udp )`

This is the function type of the user-defined collation destroy function. The only parameter is the user-data pointer passed in as the fourth parameter to `sqlite3_create_collation_v2()`.

Although collation functions have access to a user-data pointer, they don’t have an `sqlite3_context` pointer. That means there is no way to communicate an error back to the SQLite engine. As such, if you have a complex collation function, you should try to eliminate as many error sources as you can. Specifically, that means it is a good idea to pre-allocate any working buffers you might need, as there is no way to abort a comparison if your memory allocations fail. Since the collation function is really just a simple comparison, it is expected to work and provide an answer every time.

Collations can also be dynamically registered on demand. See [sqlite3_collation_needed()](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/re243.html "sqlite3_collation_needed()") in [Appendix G](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/apg.html "Appendix G. SQLite C API Reference") for more details.

## Collation Example

Here is a simple example of a user-defined collation. In this example, we’re defining a `STRINGNUM` collation that can be used to sort string representations of numeric values.

Unless they’re the same length, string representations of numbers often sort in odd ways. For example, using standard text sorting rules, the string `'485'` will sort before the string `'73'` because the character `'4'` sorts before the character`'7'`, just as the character `'D'` sorts before the character `'G'`. To be clear, these are text strings made up of characters that represent numeric digits, not actual numbers.

The collation attempts to convert these strings into a numeric representation and then use that numeric value for sorting. Using this collation, the string `'485'`will sort after `'73'`. To keep things simple, we’re only going to deal with integer values:

int col_str_num( void *udp,
        int lenA, const void *strA,
        int lenB, const void *strB )
{
    int valA = col_str_num_atoi_n( (const char*)strA, lenA );
    int valB = col_str_num_atoi_n( (const char*)strB, lenB );
    return valA - valB;
}
static int col_str_num_atoi_n( const char *str, int len )
{
    int total = 0, i;
    for ( i = 0; i < len; i++ ) {
        if ( ! isdigit( str[i] ) ) {
            break;
        }
        total *= 10;
        total += digittoint( str[i] );
    }
    return total;
}

The collation attempts to convert each string into an integer value using our custom `col_str_num_atoi_n()` function, and then compares the numeric results. The `col_` `str_` `num_atoi_n()` function is very similar to the C standard `atoi()` function, with the prime difference that it takes a maximum length parameter. That is required in this case, since the strings passed into our collations may not be null-terminated.

We would register this collation with SQLite like this:

sqlite3_create_collation( db, "STRINGNUM", SQLITE_UTF8, NULL, col_str_num );

Because the standard C function `isdigit()` is not Unicode aware, our collation sort function will only work with strings that are limited to 7-bit ASCII.

We can then have SQL that looks like this:

sqlite> **`CREATE TABLE t ( s TEXT );`**
sqlite> **`INSERT INTO t VALUES ( '485' );`**
sqlite> **`INSERT INTO t VALUES ( '73' );`**
sqlite> **`SELECT s FROM t ORDER BY s;`**
485
73
sqlite> **`SELECT s FROM t ORDER BY s COLLATE STRINGNUM;`**
73
485

It would also be possible to permanently associate our collation with a specific table column by including the collation in the table definition. See [CREATE TABLE](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/re83.html "CREATE TABLE") in [Appendix C](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/apc.html "Appendix C. SQLite SQL Command Reference") for more details.





## Name

SQLITE_ENABLE_FTS3 — Enable full-text search module

## Description

Enables full-text search module. See [Full-Text Search Module](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/ch08s03.html "Full-Text Search Module") for more details.

## Name

SQLITE_ENABLE_FTS3_PARENTHESIS — Enable FTS extended syntax

## Description

Enables the extended FTS3 query syntax, including nested parenthetical queries and the `AND` and `NOT` keywords.



## Name

SQLITE_ENABLE_ICU — Enable the ICU extension

## Description

Enables *International Components for Unicode* ([http://www.icu-project.org/](http://www.icu-project.org/)) support. This library allows SQLite to deal with non-English and non-ASCII more intelligently. Using this directive requires the ICU library to be available and linked in any build. See [ICU Internationalization Extension](https://learning.oreilly.com/library/view/using-sqlite/9781449394592/ch08s02.html "ICU Internationalization Extension") for more details.



## Name

hex() — Dump BLOB as hexadecimal

## Common Usage

hex( *`data`* )

## Description

The `hex()` function converts a BLOB value into a hexadecimal text representation. The parameter `data` is assumed to be a BLOB. If it is not a BLOB, it will be converted into one. The returned text value will contain two hexadecimal characters for each byte in the BLOB.

Be careful about using `hex()` with large BLOBs. The UTF-8 text representation is twice as big as the original BLOB value, and the UTF-16 representation is four times as large.







## 3vl operators

| value | NOT   |
| ----- | ----- |
| true  | false |
| false | true  |
| NULL  | NULL  |

| AND   | true  | false | NULL  |
| ----- | ----- | ----- | ----- |
| true  | true  | false | NULL  |
| false | false | false | false |
| NULL  | NULL  | false | NULL  |

| OR    | true | false | NULL |
| ----- | ---- | ----- | ---- |
| true  | true | true  | true |
| false | true | false | NULL |
| NULL  | true | NULL  | NULL |

=NULL -> NULL

~ bitwise inversion

|| string concatenation

IN

LIKE

GLOB

MATCH

REGEXP
