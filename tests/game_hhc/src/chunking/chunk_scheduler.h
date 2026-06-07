#ifndef HHC_CHUNK_SCHEDULER_H
#define HHC_CHUNK_SCHEDULER_H

/*!
 *  defined in 'chunking_internal.h'.
 */
typedef struct hhc_chunk_scheduler hhc_chunk_scheduler;

typedef enum chunk_scheduler_id
{
    CHUNK_SCHEDULER_ID_NONE,
    CHUNK_SCHEDULER_ID_1ST,
    CHUNK_SCHEDULER_ID_2ND,
    CHUNK_SCHEDULER_ID_3RD
} chunk_scheduler_id;

#define CHUNK_SCHEDULER_LAST_ID CHUNK_SCHEDULER_3RD_ID
#define CHUNK_SCHEDULERS_MAX 3

/*!
 *  @brief radius of a sphere of chunks, center-exclusive, that a given scheduler
 *  will be mapped onto.
 */
typedef enum chunk_scheduler_radius
{
    CHUNK_SCHEDULER_RADIUS_1ST = 4,
    CHUNK_SCHEDULER_RADIUS_2ND = 12,
    CHUNK_SCHEDULER_RADIUS_3RD = 16
} chunk_scheduler_radius;

/*!
 *  @brief budget of work given to each @ref hhc_chunk_scheduler to be consumed each frame.
 */
typedef enum chunk_scheduler_budget
{
    CHUNK_SCHEDULER_BUDGET_PRIORITY_LOW = 10000000,
    CHUNK_SCHEDULER_BUDGET_PRIORITY_MID = 15000000,
    CHUNK_SCHEDULER_BUDGET_PRIORITY_HIGH = 30000000
} chunk_scheduler_budget;

/*!
 *  @brief cost of work requested by chunks in a @ref hhc_chunk_scheduler.
 */
typedef enum chunk_work_cost
{
    CHUNK_WORK_COST_SCAN = 5,
    CHUNK_WORK_COST_PUSH = 20,
    CHUNK_WORK_COST_POP = 20,
    CHUNK_WORK_COST_IMPORT_AIR = 25,
    CHUNK_WORK_COST_IMPORT_NON_AIR = 100,
    CHUNK_WORK_COST_EXPORT = 100,
    CHUNK_WORK_COST_MESH_AIR = 2,
    CHUNK_WORK_COST_MESH_NON_AIR = 50,
    CHUNK_WORK_COST_GENERATE_AIR = 100,
    CHUNK_WORK_COST_GENERATE_NON_AIR = 500
} chunk_work_cost;

#endif /* HHC_CHUNK_SCHEDULER_H */
