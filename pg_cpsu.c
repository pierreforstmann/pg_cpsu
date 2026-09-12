/*-------------------------------------------------------------------------
 *
 * pg_cpsu
 *  
 * This program is open source, licensed under the PostgreSQL license.
 * For license terms, see the LICENSE file.
 *          
 * Copyright (c) 2026, Pierre Forstmann.
 *            
 *-------------------------------------------------------------------------
*/
#include "postgres.h"
#include "executor/executor.h"
#include "storage/proc.h"
#include "access/xact.h"

#include "tcop/tcopprot.h"
#include "tcop/utility.h"
#include "utils/guc.h"
#include "utils/snapmgr.h"

PG_MODULE_MAGIC;

/* 
 * global variable
 */
static	int action_level = INFO;

/* Saved hook values in case of unload */
static ExecutorStart_hook_type prev_ExecutorStart = NULL;
static ExecutorRun_hook_type prev_ExecutorRun = NULL;
static ExecutorFinish_hook_type prev_ExecutorFinish = NULL;

/*---- Function declarations ----*/

void		_PG_init(void);
void		_PG_fini(void);

static void cpsu_ExecutorStart(QueryDesc *queryDesc, int eflags);

static void cpsu_ExecutorRun(QueryDesc *queryDesc,
            			 ScanDirection direction,
				 long unsigned count
);
static void cpsu_ExecutorFinish(QueryDesc *queryDesc);



/*
 * Module load callback
 */
void
_PG_init(void)
{
        char    *action = NULL;

	elog(DEBUG5, "pg_cpsu:_PG_init():entry");

        DefineCustomStringVariable("pg_cpsu.action_level",
				"setting action",
				NULL,
				&action,
				NULL,
				PGC_POSTMASTER,
				0,
				NULL,
				NULL,
				NULL);
	if (action == NULL) {
		elog(LOG, "pg_cpsu:_PG_init(): missing parameter pg_cpsu_level.action_level");
                action_level = INFO;
                
	}
        else {
                if (strcmp(action, "fatal") == 0)
			action_level = FATAL;
		else if (strcmp(action, "error") == 0)
			action_level = ERROR;
		else if (strcmp(action, "warning") == 0)
			action_level = WARNING;
		else if (strcmp(action, "notice") == 0)
			action_level = NOTICE;
		else if (strcmp(action, "log") == 0)
			action_level = LOG;
		else if (strcmp(action, "info") == 0)
			action_level = INFO;
                else {
                        elog(LOG, "pg_cpsu:_PG_init(): invalid parameter pg_cpsu_level.action_level");
                        action_level = INFO;
                }
        }

	prev_ExecutorStart = ExecutorStart_hook;
	ExecutorStart_hook = cpsu_ExecutorStart;
	prev_ExecutorRun = ExecutorRun_hook;
	ExecutorRun_hook = cpsu_ExecutorRun;
	prev_ExecutorFinish = ExecutorFinish_hook;
	ExecutorFinish_hook = cpsu_ExecutorFinish;

	elog(DEBUG5, "pg_cpsu:_PG_init():exit");
}


/*
 *  Module unload callback
 */
void
_PG_fini(void)
{
	elog(DEBUG5, "pg_cpsu:_PG_fini():entry");

	ExecutorStart_hook = prev_ExecutorStart;
	ExecutorRun_hook = prev_ExecutorRun;
	ExecutorFinish_hook = prev_ExecutorFinish;

	elog(DEBUG5, "pg_cpsu:_PG_fini():entry");
}

static void
cpsu_ExecutorStart(QueryDesc *queryDesc, int eflags)
{
	uint64	queryId;
	uint64 numParams;

	elog(DEBUG5, "pg_cpsu: pglps_ExecutorStart: entry");
	queryId = queryDesc->plannedstmt->queryId;
	ereport(LOG, (errmsg("pg_cpsu: queryId=%ld", queryId)));
	if (queryDesc->params != NULL) {
		numParams = queryDesc->params->numParams;
	        ereport(action_level, 
                        (errmsg("pg_cpsu: prepared statement '%s' used (parameters number=%ld)", 
                               queryDesc->sourceText, numParams)));  
        }
	else {
		numParams = 0;
	        ereport(action_level, 
                        (errmsg("pg_cpsu: prepared statement '%s' not used (parameter number=%ld)", 
                                       queryDesc->sourceText, numParams)));
        }

	if (prev_ExecutorStart)
		prev_ExecutorStart(queryDesc, eflags);
	else
		standard_ExecutorStart(queryDesc, eflags);
	elog(DEBUG5, "pg_cpsu: cpsu_ExecutorStart: exit");
}

static void
cpsu_ExecutorRun(QueryDesc *queryDesc,
		 ScanDirection direction,
		 long unsigned count)
{
	elog(DEBUG5, "pg_cpsu: cpsu_ExecutorRun: entry");

	if (prev_ExecutorRun)
		prev_ExecutorRun(queryDesc, direction, count);
	else
		standard_ExecutorRun(queryDesc, direction, count);

	elog(DEBUG5, "pg_cpsu: cpsu_ExecutorRun: exit");
}

static void
cpsu_ExecutorFinish(QueryDesc *queryDesc)
{
	elog(DEBUG5, "pg_cpsu: cpsu_ExecutorFinish: entry");

	if (prev_ExecutorFinish)
		prev_ExecutorFinish(queryDesc);
	else
		standard_ExecutorFinish(queryDesc);

	elog(DEBUG5, "pg_cpsu: cpsu_ExecutorFinish: exit");
}
