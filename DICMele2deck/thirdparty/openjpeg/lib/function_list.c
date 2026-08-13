#include "opj_includes.h"
//Default size of the validation list, if not sufficient, data will be reallocated with a double size.
#define OPJ_VALIDATION_SIZE 10

opj_procedure_list_t *  opj_procedure_list_create()
{
    /* memory allocation */
    opj_procedure_list_t * l_validation = (opj_procedure_list_t *) calloc(1,
                                          sizeof(opj_procedure_list_t));
    if (! l_validation) {
        return 00;
    }
    /* initialization */
    l_validation->m_nb_max_procedures = OPJ_VALIDATION_SIZE;
    l_validation->m_procedures = (opj_procedure*)calloc(OPJ_VALIDATION_SIZE,
                                 sizeof(opj_procedure));
    if (! l_validation->m_procedures) {
        free(l_validation);
        return 00;
    }
    return l_validation;
}

void  opj_procedure_list_destroy(opj_procedure_list_t * p_list)
{
    if (! p_list) {
        return;
    }
    /* initialization */
    if (p_list->m_procedures) {
        free(p_list->m_procedures);
    }
    free(p_list);
}

OPJ_BOOL opj_procedure_list_add_procedure(
 opj_procedure_list_t * p_validation_list,
 opj_procedure p_procedure,
 opj_event_mgr_t* p_manager
)
{
    if (p_validation_list->m_nb_max_procedures == p_validation_list->m_nb_procedures)
    {
        opj_procedure * new_procedures;

        p_validation_list->m_nb_max_procedures += OPJ_VALIDATION_SIZE;
        new_procedures = (opj_procedure*)realloc(
                             p_validation_list->m_procedures,
                             p_validation_list->m_nb_max_procedures * sizeof(opj_procedure));
        if (! new_procedures) {
            free(p_validation_list->m_procedures);
            p_validation_list->m_nb_max_procedures = 0;
            p_validation_list->m_nb_procedures = 0;
            opj_event_msg(p_manager, EVT_ERROR,"Not enough memory to add a new validation procedure\n");
            return OPJ_FALSE;
        } else {
            p_validation_list->m_procedures = new_procedures;
        }
    }
    p_validation_list->m_procedures[p_validation_list->m_nb_procedures] = p_procedure;
    ++p_validation_list->m_nb_procedures;

    return OPJ_TRUE;
}

OPJ_UINT32 opj_procedure_list_get_nb_procedures(opj_procedure_list_t * p_validation_list)
{
    return p_validation_list->m_nb_procedures;
}

opj_procedure * opj_procedure_list_get_first_procedure(opj_procedure_list_t * p_validation_list)
{
    return p_validation_list->m_procedures;
}

void opj_procedure_list_clear(opj_procedure_list_t * p_validation_list)
{
    p_validation_list->m_nb_procedures = 0;
}
