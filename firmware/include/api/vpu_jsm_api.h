/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2020-2025, Intel Corporation.
 */

/**
 * @addtogroup Jsm
 * @{
 */

/**
 * @file
 * @brief JSM shared definitions
 */
#ifndef VPU_JSM_API_H
#define VPU_JSM_API_H

/*
 * Major version changes that break backward compatibility
 */
#define VPU_JSM_API_VER_MAJOR 3

/*
 * Minor version changes when API backward compatibility is preserved.
 */
#define VPU_JSM_API_VER_MINOR 32

/*
 * API header changed (field names, documentation, formatting) but API itself has not been changed
 */
#define VPU_JSM_API_VER_PATCH 1

/*
 * Index in the API version table
 */
#define VPU_JSM_API_VER_INDEX 4

/*
 * Number of Priority Bands for Hardware Scheduling
 * Bands: Idle(0), Normal(1), Focus(2), RealTime(3)
 */
#define VPU_HWS_NUM_PRIORITY_BANDS 4

/* Max number of impacted contexts that can be dealt with the engine reset command */
#define VPU_MAX_ENGINE_RESET_IMPACTED_CONTEXTS 3

/*
 * Pack the API structures to enforce binary compatibility
 * Align to 8 bytes for optimal performance
 */
#pragma pack(push, 8)

/*
 * Engine indexes.
 */
#define VPU_ENGINE_COMPUTE 0
#define VPU_ENGINE_NB 1

/*
 * VPU status values.
 */
#define VPU_JSM_STATUS_SUCCESS 0x0U
#define VPU_JSM_STATUS_PARSING_ERR 0x1U
#define VPU_JSM_STATUS_PROCESSING_ERR 0x2U
#define VPU_JSM_STATUS_PREEMPTED 0x3U
#define VPU_JSM_STATUS_ABORTED 0x4U
#define VPU_JSM_STATUS_USER_CTX_VIOL_ERR 0x5U
#define VPU_JSM_STATUS_GLOBAL_CTX_VIOL_ERR 0x6U
#define VPU_JSM_STATUS_MVNCI_WRONG_INPUT_FORMAT 0x7U
#define VPU_JSM_STATUS_MVNCI_UNSUPPORTED_NETWORK_ELEMENT 0x8U
#define VPU_JSM_STATUS_MVNCI_INVALID_HANDLE 0x9U
#define VPU_JSM_STATUS_MVNCI_OUT_OF_RESOURCES 0xAU
#define VPU_JSM_STATUS_MVNCI_NOT_IMPLEMENTED 0xBU
#define VPU_JSM_STATUS_MVNCI_INTERNAL_ERROR 0xCU
/* @deprecated (use VPU_JSM_STATUS_PREEMPTED_MID_COMMAND instead) */
#define VPU_JSM_STATUS_PREEMPTED_MID_INFERENCE 0xDU
/* Job status returned when the job was preempted mid-command */
#define VPU_JSM_STATUS_PREEMPTED_MID_COMMAND 0xDU
#define VPU_JSM_STATUS_MVNCI_CONTEXT_VIOLATION_HW 0xEU
#define VPU_JSM_STATUS_MVNCI_PREEMPTION_TIMED_OUT 0xFU

/*
 * Host <-> VPU IPC channels.
 * ASYNC commands use a high priority channel, other messages use low-priority ones.
 */
#define VPU_IPC_CHAN_ASYNC_CMD 0
#define VPU_IPC_CHAN_GEN_CMD 10
#define VPU_IPC_CHAN_JOB_RET 11

/*
 * Job flags bit masks.
 */
enum {
    /*
     * Null submission mask.
     * When set, batch buffer's commands are not processed but returned as
     * successful immediately, except fences and timestamps.
     * When cleared, batch buffer's commands are processed normally.
     * Used for testing and profiling purposes.
     */
    VPU_JOB_FLAGS_NULL_SUBMISSION_MASK = (1 << 0U),
    /*
     * Inline command mask.
     * When set, the object in job queue is an inline command (see struct vpu_inline_cmd below).
     * When cleared, the object in job queue is a job (see struct vpu_job below).
     */
    VPU_JOB_FLAGS_INLINE_CMD_MASK = (1 << 1U),
    /*
     * VPU private data mask.
     * Reserved for the VPU to store private data about the job (or inline command)
     * while being processed.
     */
    VPU_JOB_FLAGS_PRIVATE_DATA_MASK = 0xFFFF0000U
};

/*
 * Job queue flags bit masks.
 */
enum {
    /*
     * No job done notification mask.
     * When set, indicates that no job done notification should be sent for any
     * job from this queue. When cleared, indicates that job done notification
     * should be sent for every job completed from this queue.
     */
    VPU_JOB_QUEUE_FLAGS_NO_JOB_DONE_MASK = (1 << 0U),
    /*
     * Native fence usage mask.
     * When set, indicates that job queue uses native fences (as inline commands
     * in job queue). Such queues may also use legacy fences (as commands in batch buffers).
     * When cleared, indicates the job queue only uses legacy fences.
     * NOTES:
     *   1. For queues using native fences, VPU expects that all jobs in the queue
     *      are immediately followed by an inline command object. This object is expected
     *      to be a fence signal command in most cases, but can also be a NOP in case the host
     *      does not need per-job fence signalling. Other inline commands objects can be
     *      inserted between "job and inline command" pairs.
     *  2. Native fence queues are only supported on VPU 40xx onwards.
     */
    VPU_JOB_QUEUE_FLAGS_USE_NATIVE_FENCE_MASK = (1 << 1U),
    /*
     * Enable turbo mode for testing NPU performance; not recommended for regular usage.
     */
    VPU_JOB_QUEUE_FLAGS_TURBO_MODE = (1 << 2U),
    /*
     * Queue error detection mode flag
     * For 'interactive' queues (this bit not set), the FW will identify queues that have not
     * completed a job inside the TDR timeout as in error as part of engine reset sequence.
     * For 'non-interactive' queues (this bit set), the FW will identify queues that have not
     * progressed the heartbeat inside the non-interactive no-progress timeout as in error as
     * part of engine reset sequence. Additionally, there is an upper limit applied to these
     * queues: even if they progress the heartbeat, if they run longer than non-interactive
     * timeout, then the FW will also identify them as in error.
     */
    VPU_JOB_QUEUE_FLAGS_NON_INTERACTIVE = (1 << 3U)
};

/*
 * Max length (including trailing NULL char) of trace entity name (e.g., the
 * name of a logging destination or a loggable HW component).
 */
#define VPU_TRACE_ENTITY_NAME_MAX_LEN 32

/*
 * Max length (including trailing NULL char) of a dyndbg command.
 *
 * NOTE: 96 is used so that the size of 'struct vpu_ipc_msg' in the JSM API is
 * 128 bytes (multiple of 64 bytes, the cache line size).
 */
#define VPU_DYNDBG_CMD_MAX_LEN 96

/*
 * For HWS command queue scheduling, we can prioritise command queues inside the
 * same process with a relative in-process priority. Valid values for relative
 * priority are given below - max and min.
 */
#define VPU_HWS_COMMAND_QUEUE_MAX_IN_PROCESS_PRIORITY 7
#define VPU_HWS_COMMAND_QUEUE_MIN_IN_PROCESS_PRIORITY -7

/*
 * For HWS priority scheduling, we can have multiple realtime priority bands.
 * They are numbered 0 to a MAX.
 */
#define VPU_HWS_MAX_REALTIME_PRIORITY_LEVEL 31U

/*
 * vpu_jsm_engine_reset_context flag definitions
 */
#define VPU_ENGINE_RESET_CONTEXT_FLAG_COLLATERAL_DAMAGE_MASK (1 << 0U)
#define VPU_ENGINE_RESET_CONTEXT_HANG_PRIMARY_CAUSE 0
#define VPU_ENGINE_RESET_CONTEXT_COLLATERAL_DAMAGE 1

/*
 * Invalid command queue handle identifier. Applies to cmdq_id and cmdq_group
 * in this API.
 */
#define VPU_HWS_INVALID_CMDQ_HANDLE 0ULL

/*
 * Inline commands types.
 */
/*
 * NOP.
 * VPU does nothing other than consuming the inline command object.
 */
#define VPU_INLINE_CMD_TYPE_NOP 0x0
/*
 * Fence wait.
 * VPU waits for the fence current value to reach monitored value.
 * Fence wait operations are executed upon job dispatching. While waiting for
 * the fence to be satisfied, VPU blocks fetching of the next objects in the queue.
 * Jobs present in the queue prior to the fence wait object may be processed
 * concurrently.
 */
#define VPU_INLINE_CMD_TYPE_FENCE_WAIT 0x1
/*
 * Fence signal.
 * VPU sets the fence current value to the provided value. If new current value
 * is equal to or higher than monitored value, VPU sends fence signalled notification
 * to the host. Fence signal operations are executed upon completion of all the jobs
 * present in the queue prior to them, and in-order relative to each other in the queue.
 * But jobs in-between them may be processed concurrently and may complete out-of-order.
 */
#define VPU_INLINE_CMD_TYPE_FENCE_SIGNAL 0x2

/*
 * Job scheduling priority bands for both hardware scheduling and OS scheduling.
 */
enum vpu_job_scheduling_priority_band {
    VPU_JOB_SCHEDULING_PRIORITY_BAND_IDLE = 0,
    VPU_JOB_SCHEDULING_PRIORITY_BAND_NORMAL = 1,
    VPU_JOB_SCHEDULING_PRIORITY_BAND_FOCUS = 2,
    VPU_JOB_SCHEDULING_PRIORITY_BAND_REALTIME = 3,
    VPU_JOB_SCHEDULING_PRIORITY_BAND_COUNT = 4,
};

/*
 * Job format.
 * Jobs defines the actual workloads to be executed by a given engine.
 */
struct vpu_job {
    /**< Address of VPU commands batch buffer */
    volatile uint64_t batch_buf_addr;
    /**< Job ID */
    volatile uint32_t job_id;
    /**< Flags bit field, see VPU_JOB_FLAGS_* above */
    volatile uint32_t flags;
    /**
     * Doorbell ring timestamp taken by KMD from SoC's global system clock, in
     * microseconds. NPU can convert this value to its own fixed clock's timebase,
     * to match other profiling timestamps.
     */
    volatile uint64_t doorbell_timestamp;
    /**< Extra id for job tracking, used only in the firmware perf traces */
    volatile uint64_t host_tracking_id;
    /**< Address of the primary preemption buffer to use for this job */
    volatile uint64_t primary_preempt_buf_addr;
    /**< Size of the primary preemption buffer to use for this job */
    volatile uint32_t primary_preempt_buf_size;
    /**< Size of secondary preemption buffer to use for this job */
    volatile uint32_t secondary_preempt_buf_size;
    /**< Address of secondary preemption buffer to use for this job */
    volatile uint64_t secondary_preempt_buf_addr;
    uint64_t reserved_0;
};
typedef struct vpu_job vpu_job_t;

/*
 * Inline command format.
 * Inline commands are the commands executed at scheduler level (typically,
 * synchronization directives). Inline command and job objects must be of
 * the same size and have flags field at same offset.
 */
struct vpu_inline_cmd {
    uint64_t reserved_0;
    /* Inline command type, see VPU_INLINE_CMD_TYPE_* defines. */
    volatile uint32_t type;
    /* Flags bit field, see VPU_JOB_FLAGS_* above. */
    volatile uint32_t flags;
    /* Inline command payload. Depends on inline command type. */
    union {
        /* Fence (wait and signal) commands' payload. */
        struct {
            /* Fence object handle. */
            volatile uint64_t fence_handle;
            /* User VA of the current fence value. */
            volatile uint64_t current_value_va;
            /* User VA of the monitored fence value (read-only). */
            volatile uint64_t monitored_value_va;
            /* Value to wait for or write in fence location. */
            volatile uint64_t value;
            /* User VA of the log buffer in which to add log entry on completion. */
            volatile uint64_t log_buffer_va;
            /* NPU private data. */
            volatile uint64_t npu_private_data;
        } fence;
        /* Other commands do not have a payload. */
        /* Payload definition for future inline commands can be inserted here. */
        uint64_t reserved_1[6];
    } payload;
};

/*
 * Job queue slots can be populated either with job objects or inline command objects.
 */
union vpu_jobq_slot {
    struct vpu_job job;
    struct vpu_inline_cmd inline_cmd;
};

/*
 * Job queue control registers.
 */
struct vpu_job_queue_header {
    volatile uint32_t engine_idx;
    uint32_t head;
    volatile uint32_t tail;
    volatile uint32_t flags;
    /* Set to 1 to indicate priority_band field is valid */
    uint32_t priority_band_valid;
    /*
     * Priority for the work of this job queue, valid only if the HWS is NOT used
     * and the `priority_band_valid` is set to 1. It is applied only during
     * the VPU_IPC_MSG_REGISTER_DB message processing.
     * The device firmware might use the `priority_band` to optimize the power
     * management logic, but it will not affect the order of jobs.
     * Available priority bands: @see enum vpu_job_scheduling_priority_band
     */
    uint32_t priority_band;
    /* Inside realtime band assigns a further priority, limited to 0..31 range */
    uint32_t realtime_priority_level;
    uint32_t reserved_0[9];
};
typedef struct vpu_job_queue_header vpu_job_queue_header_t;

/*
 * Job queue format.
 */
struct vpu_job_queue {
    struct vpu_job_queue_header header;
    union vpu_jobq_slot slot[];
};
typedef struct vpu_job_queue vpu_job_queue_t;

/**
 * Logging entity types.
 *
 * This enum defines the different types of entities involved in logging.
 */
enum vpu_trace_entity_type {
    /** Logging destination (entity where logs can be stored / printed). */
    VPU_TRACE_ENTITY_TYPE_DESTINATION = 1,
    /** Loggable HW component (HW entity that can be logged). */
    VPU_TRACE_ENTITY_TYPE_HW_COMPONENT = 2,
};

/*
 * HWS specific log buffer header details.
 * Total size is 32 bytes.
 */
struct vpu_hws_log_buffer_header {
    /* Written by VPU after adding a log entry. Initialised by host to 0. */
    uint32_t first_free_entry_index;
    /* Incremented by VPU every time the VPU writes the 0th entry; initialised by host to 0. */
    uint32_t wraparound_count;
    /*
     * This is the number of buffers that can be stored in the log buffer provided by the host.
     * It is written by host before passing buffer to VPU. VPU should consider it read-only.
     */
    uint64_t num_of_entries;
    uint64_t reserved[2];
};

/*
 * HWS specific log buffer entry details.
 * Total size is 32 bytes.
 */
struct vpu_hws_log_buffer_entry {
    /* VPU timestamp must be an invariant timer tick (not impacted by DVFS) */
    uint64_t vpu_timestamp;
    /*
     * Operation type:
     *     0 - context state change
     *     1 - queue new work
     *     2 - queue unwait sync object
     *     3 - queue no more work
     *     4 - queue wait sync object
     */
    uint32_t operation_type;
    uint32_t reserved;
    /* Operation data depends on operation type */
    uint64_t operation_data[2];
};

/* Native fence log buffer types. */
enum vpu_hws_native_fence_log_type {
    VPU_HWS_NATIVE_FENCE_LOG_TYPE_WAITS = 1,
    VPU_HWS_NATIVE_FENCE_LOG_TYPE_SIGNALS = 2
};

/* HWS native fence log buffer header. */
struct vpu_hws_native_fence_log_header {
    union {
        struct {
            /* Index of the first free entry in buffer. */
            uint32_t first_free_entry_idx;
            /* Incremented each time NPU wraps around the buffer to write next entry. */
            uint32_t wraparound_count;
        };
        /* Field allowing atomic update of both fields above. */
        uint64_t atomic_wraparound_and_entry_idx;
    };
    /* Log buffer type, see enum vpu_hws_native_fence_log_type. */
    uint64_t type;
    /* Allocated number of entries in the log buffer. */
    uint64_t entry_nb;
    uint64_t reserved[2];
};

/* Native fence log operation types. */
enum vpu_hws_native_fence_log_op {
    VPU_HWS_NATIVE_FENCE_LOG_OP_SIGNAL_EXECUTED = 0,
    VPU_HWS_NATIVE_FENCE_LOG_OP_WAIT_UNBLOCKED = 1
};

/* HWS native fence log entry. */
struct vpu_hws_native_fence_log_entry {
    /* Newly signaled/unblocked fence value. */
    uint64_t fence_value;
    /* Native fence object handle to which this operation belongs. */
    uint64_t fence_handle;
    /* Operation type, see enum vpu_hws_native_fence_log_op. */
    uint64_t op_type;
    uint64_t reserved_0;
    /*
     * VPU_HWS_NATIVE_FENCE_LOG_OP_WAIT_UNBLOCKED only: Timestamp at which fence
     * wait was started (in NPU SysTime).
     */
    uint64_t fence_wait_start_ts;
    uint64_t reserved_1;
    /* Timestamp at which fence operation was completed (in NPU SysTime). */
    uint64_t fence_end_ts;
};

/* Native fence log buffer. */
struct vpu_hws_native_fence_log_buffer {
    struct vpu_hws_native_fence_log_header header;
    struct vpu_hws_native_fence_log_entry entry[];
};

/*
 * Host <-> VPU IPC messages types.
 */
enum vpu_ipc_msg_type {
    VPU_IPC_MSG_UNKNOWN = 0xFFFFFFFF,

    /* IPC Host -> Device, Async commands */
    VPU_IPC_MSG_ASYNC_CMD = 0x1100,
    VPU_IPC_MSG_ENGINE_RESET = VPU_IPC_MSG_ASYNC_CMD,
    /**
     * Preempt engine. The NPU stops (preempts) all the jobs currently
     * executing on the target engine making the engine become idle and ready to
     * execute new jobs.
     * NOTE: The NPU does not remove unstarted jobs (if any) from job queues of
     * the target engine, but it stops processing them (until the queue doorbell
     * is rung again); the host is responsible to reset the job queue, either
     * after preemption or when resubmitting jobs to the queue.
     */
    VPU_IPC_MSG_ENGINE_PREEMPT = 0x1101,
    VPU_IPC_MSG_REGISTER_DB = 0x1102,
    VPU_IPC_MSG_UNREGISTER_DB = 0x1103,
    VPU_IPC_MSG_QUERY_ENGINE_HB = 0x1104,
    VPU_IPC_MSG_GET_POWER_LEVEL_COUNT = 0x1105,
    VPU_IPC_MSG_GET_POWER_LEVEL = 0x1106,
    VPU_IPC_MSG_SET_POWER_LEVEL = 0x1107,
    /* @deprecated */
    VPU_IPC_MSG_METRIC_STREAMER_OPEN = 0x1108,
    /* @deprecated */
    VPU_IPC_MSG_METRIC_STREAMER_CLOSE = 0x1109,
    /** Configure logging (used to modify configuration passed in boot params). */
    VPU_IPC_MSG_TRACE_SET_CONFIG = 0x110a,
    /** Return current logging configuration. */
    VPU_IPC_MSG_TRACE_GET_CONFIG = 0x110b,
    /**
     * Get masks of destinations and HW components supported by the firmware
     * (may vary between HW generations and FW compile
     * time configurations)
     */
    VPU_IPC_MSG_TRACE_GET_CAPABILITY = 0x110c,
    /** Get the name of a destination or HW component. */
    VPU_IPC_MSG_TRACE_GET_NAME = 0x110d,
    /**
     * Release resource associated with host ssid . All jobs that belong to the host_ssid
     * aborted and removed from internal scheduling queues. All doorbells assigned
     * to the host_ssid are unregistered and any internal FW resources belonging to
     * the host_ssid are released.
     */
    VPU_IPC_MSG_SSID_RELEASE = 0x110e,
    /**
     * Start collecting metric data.
     * @see vpu_jsm_metric_streamer_start
     */
    VPU_IPC_MSG_METRIC_STREAMER_START = 0x110f,
    /**
     * Stop collecting metric data. This command will return success if it is called
     * for a metric stream that has already been stopped or was never started.
     * @see vpu_jsm_metric_streamer_stop
     */
    VPU_IPC_MSG_METRIC_STREAMER_STOP = 0x1110,
    /**
     * Update current and next buffer for metric data collection. This command can
     * also be used to request information about the number of collected samples
     * and the amount of data written to the buffer.
     * @see vpu_jsm_metric_streamer_update
     */
    VPU_IPC_MSG_METRIC_STREAMER_UPDATE = 0x1111,
    /**
     * Request description of selected metric groups and metric counters within
     * each group. The VPU will write the description of groups and counters to
     * the buffer specified in the command structure.
     * @see vpu_jsm_metric_streamer_start
     */
    VPU_IPC_MSG_METRIC_STREAMER_INFO = 0x1112,
    /** Control command: Priority band setup */
    VPU_IPC_MSG_SET_PRIORITY_BAND_SETUP = 0x1113,
    /** Control command: Create command queue */
    VPU_IPC_MSG_CREATE_CMD_QUEUE = 0x1114,
    /** Control command: Destroy command queue */
    VPU_IPC_MSG_DESTROY_CMD_QUEUE = 0x1115,
    /** Control command: Set context scheduling properties */
    VPU_IPC_MSG_SET_CONTEXT_SCHED_PROPERTIES = 0x1116,
    /*
     * Register a doorbell to notify VPU of new work. The doorbell may later be
     * deallocated or reassigned to another context.
     */
    VPU_IPC_MSG_HWS_REGISTER_DB = 0x1117,
    /** Control command: Log buffer setting */
    VPU_IPC_MSG_HWS_SET_SCHEDULING_LOG = 0x1118,
    /* Control command: Suspend command queue. */
    VPU_IPC_MSG_HWS_SUSPEND_CMDQ = 0x1119,
    /* Control command: Resume command queue */
    VPU_IPC_MSG_HWS_RESUME_CMDQ = 0x111a,
    /* Control command: Resume engine after reset */
    VPU_IPC_MSG_HWS_ENGINE_RESUME = 0x111b,
    /* Control command: Enable survivability/DCT mode */
    VPU_IPC_MSG_DCT_ENABLE = 0x111c,
    /* Control command: Disable survivability/DCT mode */
    VPU_IPC_MSG_DCT_DISABLE = 0x111d,
    /**
     * Dump VPU state. To be used for debug purposes only.
     * NOTE: Please introduce new ASYNC commands before this one. *
     */
    VPU_IPC_MSG_STATE_DUMP = 0x11FF,

    /* IPC Host -> Device, General commands */
    VPU_IPC_MSG_GENERAL_CMD = 0x1200,
    VPU_IPC_MSG_BLOB_DEINIT_DEPRECATED = VPU_IPC_MSG_GENERAL_CMD,
    /**
     * Control dyndbg behavior by executing a dyndbg command; equivalent to
     * Linux command:
     * @verbatim echo '<dyndbg_cmd>' > <debugfs>/dynamic_debug/control @endverbatim
     */
    VPU_IPC_MSG_DYNDBG_CONTROL = 0x1201,
    /**
     * Perform the save procedure for the D0i3 entry
     */
    VPU_IPC_MSG_PWR_D0I3_ENTER = 0x1202,

    /* IPC Device -> Host, Job completion */
    VPU_IPC_MSG_JOB_DONE = 0x2100,
    /* IPC Device -> Host, Fence signalled */
    VPU_IPC_MSG_NATIVE_FENCE_SIGNALLED = 0x2101,

    /* IPC Device -> Host, Async command completion */
    VPU_IPC_MSG_ASYNC_CMD_DONE = 0x2200,
    VPU_IPC_MSG_ENGINE_RESET_DONE = VPU_IPC_MSG_ASYNC_CMD_DONE,
    VPU_IPC_MSG_ENGINE_PREEMPT_DONE = 0x2201,
    VPU_IPC_MSG_REGISTER_DB_DONE = 0x2202,
    VPU_IPC_MSG_UNREGISTER_DB_DONE = 0x2203,
    VPU_IPC_MSG_QUERY_ENGINE_HB_DONE = 0x2204,
    VPU_IPC_MSG_GET_POWER_LEVEL_COUNT_DONE = 0x2205,
    VPU_IPC_MSG_GET_POWER_LEVEL_DONE = 0x2206,
    VPU_IPC_MSG_SET_POWER_LEVEL_DONE = 0x2207,
    /* @deprecated */
    VPU_IPC_MSG_METRIC_STREAMER_OPEN_DONE = 0x2208,
    /* @deprecated */
    VPU_IPC_MSG_METRIC_STREAMER_CLOSE_DONE = 0x2209,
    /** Response to VPU_IPC_MSG_TRACE_SET_CONFIG. */
    VPU_IPC_MSG_TRACE_SET_CONFIG_RSP = 0x220a,
    /** Response to VPU_IPC_MSG_TRACE_GET_CONFIG. */
    VPU_IPC_MSG_TRACE_GET_CONFIG_RSP = 0x220b,
    /** Response to VPU_IPC_MSG_TRACE_GET_CAPABILITY. */
    VPU_IPC_MSG_TRACE_GET_CAPABILITY_RSP = 0x220c,
    /** Response to VPU_IPC_MSG_TRACE_GET_NAME. */
    VPU_IPC_MSG_TRACE_GET_NAME_RSP = 0x220d,
    /** Response to VPU_IPC_MSG_SSID_RELEASE. */
    VPU_IPC_MSG_SSID_RELEASE_DONE = 0x220e,
    /**
     * Response to VPU_IPC_MSG_METRIC_STREAMER_START.
     * VPU will return an error result if metric collection cannot be started,
     * e.g. when the specified metric mask is invalid.
     * @see vpu_jsm_metric_streamer_done
     */
    VPU_IPC_MSG_METRIC_STREAMER_START_DONE = 0x220f,
    /**
     * Response to VPU_IPC_MSG_METRIC_STREAMER_STOP.
     * Returns information about collected metric data.
     * @see vpu_jsm_metric_streamer_done
     */
    VPU_IPC_MSG_METRIC_STREAMER_STOP_DONE = 0x2210,
    /**
     * Response to VPU_IPC_MSG_METRIC_STREAMER_UPDATE.
     * Returns information about collected metric data.
     * @see vpu_jsm_metric_streamer_done
     */
    VPU_IPC_MSG_METRIC_STREAMER_UPDATE_DONE = 0x2211,
    /**
     * Response to VPU_IPC_MSG_METRIC_STREAMER_INFO.
     * Returns a description of the metric groups and metric counters.
     * @see vpu_jsm_metric_streamer_done
     */
    VPU_IPC_MSG_METRIC_STREAMER_INFO_DONE = 0x2212,
    /**
     * Asynchronous event sent from the VPU to the host either when the current
     * metric buffer is full or when the VPU has collected a multiple of
     * @ref vpu_jsm_metric_streamer_start::notify_sample_count samples as indicated
     * through the start command (VPU_IPC_MSG_METRIC_STREAMER_START). Returns
     * information about collected metric data.
     * @see vpu_jsm_metric_streamer_done
     */
    VPU_IPC_MSG_METRIC_STREAMER_NOTIFICATION = 0x2213,
    /** Response to control command: Priority band setup */
    VPU_IPC_MSG_SET_PRIORITY_BAND_SETUP_RSP = 0x2214,
    /** Response to control command: Create command queue */
    VPU_IPC_MSG_CREATE_CMD_QUEUE_RSP = 0x2215,
    /** Response to control command: Destroy command queue */
    VPU_IPC_MSG_DESTROY_CMD_QUEUE_RSP = 0x2216,
    /** Response to control command: Set context scheduling properties */
    VPU_IPC_MSG_SET_CONTEXT_SCHED_PROPERTIES_RSP = 0x2217,
    /** Response to control command: Log buffer setting */
    VPU_IPC_MSG_HWS_SET_SCHEDULING_LOG_RSP = 0x2218,
    /* IPC Device -> Host, HWS notify index entry of log buffer written */
    VPU_IPC_MSG_HWS_SCHEDULING_LOG_NOTIFICATION = 0x2219,
    /* IPC Device -> Host, HWS completion of a context suspend request */
    VPU_IPC_MSG_HWS_SUSPEND_CMDQ_DONE = 0x221a,
    /* Response to control command: Resume command queue */
    VPU_IPC_MSG_HWS_RESUME_CMDQ_RSP = 0x221b,
    /* Response to control command: Resume engine command response */
    VPU_IPC_MSG_HWS_RESUME_ENGINE_DONE = 0x221c,
    /* Response to control command: Enable survivability/DCT mode */
    VPU_IPC_MSG_DCT_ENABLE_DONE = 0x221d,
    /* Response to control command: Disable survivability/DCT mode */
    VPU_IPC_MSG_DCT_DISABLE_DONE = 0x221e,
    /**
     * Response to state dump control command.
     * NOTE: Please introduce new ASYNC responses before this one. *
     */
    VPU_IPC_MSG_STATE_DUMP_RSP = 0x22FF,

    /* IPC Device -> Host, General command completion */
    VPU_IPC_MSG_GENERAL_CMD_DONE = 0x2300,
    VPU_IPC_MSG_BLOB_DEINIT_DONE = VPU_IPC_MSG_GENERAL_CMD_DONE,
    /** Response to VPU_IPC_MSG_DYNDBG_CONTROL. */
    VPU_IPC_MSG_DYNDBG_CONTROL_RSP = 0x2301,
    /**
     * Acknowledgment of completion of the save procedure initiated by
     * VPU_IPC_MSG_PWR_D0I3_ENTER
     */
    VPU_IPC_MSG_PWR_D0I3_ENTER_DONE = 0x2302,
};
typedef enum vpu_ipc_msg_type vpu_ipc_msg_type_t;

enum vpu_ipc_msg_status { VPU_IPC_MSG_FREE, VPU_IPC_MSG_ALLOCATED };
typedef enum vpu_ipc_msg_status vpu_ipc_msg_status_t;

/*
 * Host <-> LRT IPC message payload definitions
 */
struct vpu_ipc_msg_payload_engine_reset {
    /* Engine to be reset. */
    uint32_t engine_idx;
    /* Reserved */
    uint32_t reserved_0;
};
typedef struct vpu_ipc_msg_payload_engine_reset vpu_ipc_msg_payload_engine_reset_t;

struct vpu_ipc_msg_payload_engine_preempt {
    /* Engine to be preempted. */
    uint32_t engine_idx;
    /* ID of the preemption request. */
    uint32_t preempt_id;
};
typedef struct vpu_ipc_msg_payload_engine_preempt vpu_ipc_msg_payload_engine_preempt_t;

/*
 * @brief Register doorbell command structure.
 * This structure supports doorbell registration for only OS scheduling.
 * @see VPU_IPC_MSG_REGISTER_DB
 */
struct vpu_ipc_msg_payload_register_db {
    /* Index of the doorbell to register. */
    uint32_t db_idx;
    /* Reserved */
    uint32_t reserved_0;
    /* Virtual address in Global GTT pointing to the start of job queue. */
    uint64_t jobq_base;
    /* Size of the job queue in bytes. */
    uint32_t jobq_size;
    /* Host sub-stream ID for the context assigned to the doorbell. */
    uint32_t host_ssid;
};
typedef struct vpu_ipc_msg_payload_register_db vpu_ipc_msg_payload_register_db_t;

/**
 * @brief Unregister doorbell command structure.
 * Request structure to unregister a doorbell for both HW and OS scheduling.
 * @see VPU_IPC_MSG_UNREGISTER_DB
 */
struct vpu_ipc_msg_payload_unregister_db {
    /* Index of the doorbell to unregister. */
    uint32_t db_idx;
    /* Reserved */
    uint32_t reserved_0;
};
typedef struct vpu_ipc_msg_payload_unregister_db vpu_ipc_msg_payload_unregister_db_t;

struct vpu_ipc_msg_payload_query_engine_hb {
    /* Engine to return heartbeat value. */
    uint32_t engine_idx;
    /* Reserved */
    uint32_t reserved_0;
};
typedef struct vpu_ipc_msg_payload_query_engine_hb vpu_ipc_msg_payload_query_engine_hb_t;

struct vpu_ipc_msg_payload_power_level {
    /**
     * Requested power level. The power level value is in the
     * range [0, power_level_count-1] where power_level_count
     * is the number of available power levels as returned by
     * the get power level count command. A power level of 0
     * corresponds to the maximum possible power level, while
     * power_level_count-1 corresponds to the minimum possible
     * power level. Values outside of this range are not
     * considered to be valid.
     */
    uint32_t power_level;
    /* Reserved */
    uint32_t reserved_0;
};
typedef struct vpu_ipc_msg_payload_set_power_level vpu_ipc_msg_payload_set_power_level_t;

struct vpu_ipc_msg_payload_ssid_release {
    /* Host sub-stream ID for the context to be released. */
    uint32_t host_ssid;
    /* Reserved */
    uint32_t reserved_0;
};
typedef struct vpu_ipc_msg_payload_ssid_release vpu_ipc_msg_payload_ssid_release_t;

/**
 * @brief Metric streamer start command structure.
 * This structure is also used with VPU_IPC_MSG_METRIC_STREAMER_INFO to request metric
 * groups and metric counters description from the firmware.
 * @see VPU_IPC_MSG_METRIC_STREAMER_START
 * @see VPU_IPC_MSG_METRIC_STREAMER_INFO
 */
struct vpu_jsm_metric_streamer_start {
    /**
     * Bitmask to select the desired metric groups.
     * A metric group can belong only to one metric streamer instance at a time.
     * Since each metric streamer instance has a unique set of metric groups, it
     * can also identify a metric streamer instance if more than one instance was
     * started. If the VPU device does not support multiple metric streamer instances,
     * then VPU_IPC_MSG_METRIC_STREAMER_START will return an error even if the second
     * instance has different groups to the first.
     */
    uint64_t metric_group_mask;
    /** Sampling rate in nanoseconds. */
    uint64_t sampling_rate;
    /**
     * If > 0 the VPU will send a VPU_IPC_MSG_METRIC_STREAMER_NOTIFICATION message
     * after every @ref notify_sample_count samples is collected or dropped by the VPU.
     * If set to UINT_MAX the VPU will only generate a notification when the metric
     * buffer is full. If set to 0 the VPU will never generate a notification.
     */
    uint32_t notify_sample_count;
    uint32_t reserved_0;
    /**
     * Address and size of the buffer where the VPU will write metric data. The
     * VPU writes all counters from enabled metric groups one after another. If
     * there is no space left to write data at the next sample period the VPU
     * will switch to the next buffer (@ref next_buffer_addr) and will optionally
     * send a notification to the host driver if @ref notify_sample_count is non-zero.
     * If @ref next_buffer_addr is NULL the VPU will stop collecting metric data.
     */
    uint64_t buffer_addr;
    uint64_t buffer_size;
    /**
     * Address and size of the next buffer to write metric data to after the initial
     * buffer is full. If the address is NULL the VPU will stop collecting metric
     * data.
     */
    uint64_t next_buffer_addr;
    uint64_t next_buffer_size;
};
typedef struct vpu_jsm_metric_streamer_start vpu_jsm_metric_streamer_start_t;

/**
 * @brief Metric streamer stop command structure.
 * @see VPU_IPC_MSG_METRIC_STREAMER_STOP
 */
struct vpu_jsm_metric_streamer_stop {
    /** Bitmask to select the desired metric groups. */
    uint64_t metric_group_mask;
};
typedef struct vpu_jsm_metric_streamer_stop vpu_jsm_metric_streamer_stop_t;

/**
 * Provide VPU FW with buffers to write metric data.
 * @see VPU_IPC_MSG_METRIC_STREAMER_UPDATE
 */
struct vpu_jsm_metric_streamer_update {
    /** Metric group mask that identifies metric streamer instance. */
    uint64_t metric_group_mask;
    /**
     * Address and size of the buffer where the VPU will write metric data.
     * This member dictates how the update operation should perform:
     * 1. client needs information about the number of collected samples and the
     *   amount of data written to the current buffer
     * 2. client wants to switch to a new buffer
     *
     * Case 1. is identified by the buffer address being 0 or the same as the
     * currently used buffer address. In this case the buffer size is ignored and
     * the size of the current buffer is unchanged. The VPU will return an update
     * in the vpu_jsm_metric_streamer_done structure. The internal writing position
     * into the buffer is not changed.
     *
     * Case 2. is identified by the address being non-zero and differs from the
     * current buffer address. The VPU will immediately switch data collection to
     * the new buffer. Then the VPU will return an update in the
     * vpu_jsm_metric_streamer_done structure.
     */
    uint64_t buffer_addr;
    uint64_t buffer_size;
    /**
     * Address and size of the next buffer to write metric data after the initial
     * buffer is full. If the address is NULL the VPU will stop collecting metric
     * data but will continue to record dropped samples.
     *
     * Note that there is a hazard possible if both buffer_addr and the next_buffer_addr
     * are non-zero in same update request. It is the host's responsibility to ensure
     * that both addresses make sense even if the VPU just switched to writing samples
     * from the current to the next buffer.
     */
    uint64_t next_buffer_addr;
    uint64_t next_buffer_size;
};
typedef struct vpu_jsm_metric_streamer_update vpu_jsm_metric_streamer_update_t;

struct vpu_ipc_msg_payload_job_done {
    /* Engine to which the job was submitted. */
    uint32_t engine_idx;
    /* Index of the doorbell to which the job was submitted */
    uint32_t db_idx;
    /* ID of the completed job */
    uint32_t job_id;
    /* Status of the completed job */
    uint32_t job_status;
    /* Host SSID */
    uint32_t host_ssid;
    /* Zero Padding */
    uint32_t reserved_0;
    /* Command queue id */
    uint64_t cmdq_id;
};
typedef struct vpu_ipc_msg_payload_job_done vpu_ipc_msg_payload_job_done_t;

/*
 * Notification message upon native fence signalling.
 * @see VPU_IPC_MSG_NATIVE_FENCE_SIGNALLED
 */
struct vpu_ipc_msg_payload_native_fence_signalled {
    /* Engine ID. */
    uint32_t engine_idx;
    /* Host SSID. */
    uint32_t host_ssid;
    /* CMDQ ID */
    uint64_t cmdq_id;
    /* Fence object handle. */
    uint64_t fence_handle;
};

struct vpu_jsm_engine_reset_context {
    /* Host SSID */
    uint32_t host_ssid;
    /* Zero Padding */
    uint32_t reserved_0;
    /* Command queue id */
    uint64_t cmdq_id;
    /* See VPU_ENGINE_RESET_CONTEXT_* defines */
    uint64_t flags;
};

struct vpu_ipc_msg_payload_engine_reset_done {
    /* Engine ordinal */
    uint32_t engine_idx;
    /* Number of impacted contexts */
    uint32_t num_impacted_contexts;
    /* Array of impacted command queue ids and their flags */
    struct vpu_jsm_engine_reset_context impacted_contexts[VPU_MAX_ENGINE_RESET_IMPACTED_CONTEXTS];
};
typedef struct vpu_ipc_msg_payload_engine_reset_done vpu_ipc_msg_payload_engine_reset_done_t;

struct vpu_ipc_msg_payload_engine_preempt_done {
    /* Engine preempted. */
    uint32_t engine_idx;
    /* ID of the preemption request. */
    uint32_t preempt_id;
};
typedef struct vpu_ipc_msg_payload_engine_preempt_done vpu_ipc_msg_payload_engine_preempt_done_t;

/**
 * Response structure for register doorbell command for both OS
 * and HW scheduling.
 * @see VPU_IPC_MSG_REGISTER_DB
 * @see VPU_IPC_MSG_HWS_REGISTER_DB
 */
struct vpu_ipc_msg_payload_register_db_done {
    /* Index of the registered doorbell. */
    uint32_t db_idx;
    /* Reserved */
    uint32_t reserved_0;
};
typedef struct vpu_ipc_msg_payload_register_db_done vpu_ipc_msg_payload_register_db_done_t;

/**
 * Response structure for unregister doorbell command for both OS
 * and HW scheduling.
 * @see VPU_IPC_MSG_UNREGISTER_DB
 */
struct vpu_ipc_msg_payload_unregister_db_done {
    /* Index of the unregistered doorbell. */
    uint32_t db_idx;
    /* Reserved */
    uint32_t reserved_0;
};
typedef struct vpu_ipc_msg_payload_unregister_db_done vpu_ipc_msg_payload_unregister_db_done_t;

struct vpu_ipc_msg_payload_query_engine_hb_done {
    /* Engine returning heartbeat value. */
    uint32_t engine_idx;
    /* Reserved */
    uint32_t reserved_0;
    /* Heartbeat value. */
    uint64_t heartbeat;
};
typedef struct vpu_ipc_msg_payload_query_engine_hb_done vpu_ipc_msg_payload_query_engine_hb_done_t;

struct vpu_ipc_msg_payload_get_power_level_count_done {
    /**
     * Number of supported power levels. The maximum possible
     * value of power_level_count is 16 but this may vary across
     * implementations.
     */
    uint32_t power_level_count;
    /* Reserved */
    uint32_t reserved_0;
    /**
     * Power consumption limit for each supported power level in
     * [0-100%] range relative to power level 0.
     */
    uint8_t power_limit[16];
};
typedef struct vpu_ipc_msg_payload_get_power_level_count_done vpu_ipc_msg_payload_get_power_level_count_done_t;

/* HWS priority band setup request / response */
struct vpu_ipc_msg_payload_hws_priority_band_setup {
    /*
     * Grace period in 100ns units when preempting another priority band for
     * this priority band
     */
    uint32_t grace_period[VPU_HWS_NUM_PRIORITY_BANDS];
    /*
     * Default quantum in 100ns units for scheduling across processes
     * within a priority band
     * Minimum value supported by NPU is 1ms (10000 in 100ns units).
     */
    uint32_t process_quantum[VPU_HWS_NUM_PRIORITY_BANDS];
    /*
     * Default grace period in 100ns units for processes that preempt each
     * other within a priority band
     */
    uint32_t process_grace_period[VPU_HWS_NUM_PRIORITY_BANDS];
    /*
     * For normal priority band, specifies the target VPU percentage
     * in situations when it's starved by the focus band.
     */
    uint32_t normal_band_percentage;
    /*
     * TDR timeout value in milliseconds. Default value of 0 meaning no timeout.
     */
    uint32_t tdr_timeout;
    /* Non-interactive queue timeout for no progress of heartbeat in milliseconds.
     * Default value of 0 meaning no timeout.
     */
    uint32_t non_interactive_no_progress_timeout;
    /*
     * Non-interactive queue upper limit timeout value in milliseconds. Default
     * value of 0 meaning no timeout.
     */
    uint32_t non_interactive_timeout;
};

/*
 * @brief HWS create command queue request.
 * Host will create a command queue via this command.
 * Note: Cmdq group is a handle of an object which
 * may contain one or more command queues.
 * @see VPU_IPC_MSG_CREATE_CMD_QUEUE
 * @see VPU_IPC_MSG_CREATE_CMD_QUEUE_RSP
 */
struct vpu_ipc_msg_payload_hws_create_cmdq {
    /* Process id */
    uint64_t process_id;
    /* Host SSID */
    uint32_t host_ssid;
    /* Engine for which queue is being created */
    uint32_t engine_idx;
    /* Cmdq group: only used for HWS logging of state changes */
    uint64_t cmdq_group;
    /* Command queue id */
    uint64_t cmdq_id;
    /* Command queue base */
    uint64_t cmdq_base;
    /* Command queue size */
    uint32_t cmdq_size;
    /* Zero padding */
    uint32_t reserved_0;
};

/*
 * @brief HWS create command queue response.
 * @see VPU_IPC_MSG_CREATE_CMD_QUEUE
 * @see VPU_IPC_MSG_CREATE_CMD_QUEUE_RSP
 */
struct vpu_ipc_msg_payload_hws_create_cmdq_rsp {
    /* Process id */
    uint64_t process_id;
    /* Host SSID */
    uint32_t host_ssid;
    /* Engine for which queue is being created */
    uint32_t engine_idx;
    /* Command queue group */
    uint64_t cmdq_group;
    /* Command queue id */
    uint64_t cmdq_id;
};

/* HWS destroy command queue request / response */
struct vpu_ipc_msg_payload_hws_destroy_cmdq {
    /* Host SSID */
    uint32_t host_ssid;
    /* Zero Padding */
    uint32_t reserved;
    /* Command queue id */
    uint64_t cmdq_id;
};

/* HWS set context scheduling properties request / response */
struct vpu_ipc_msg_payload_hws_set_context_sched_properties {
    /* Host SSID */
    uint32_t host_ssid;
    /* Zero Padding */
    uint32_t reserved_0;
    /* Command queue id */
    uint64_t cmdq_id;
    /*
     * Priority band to assign to work of this context.
     * Available priority bands: @see enum vpu_job_scheduling_priority_band
     */
    uint32_t priority_band;
    /* Inside realtime band assigns a further priority */
    uint32_t realtime_priority_level;
    /* Priority relative to other contexts in the same process */
    int32_t in_process_priority;
    /* Zero padding / Reserved */
    uint32_t reserved_1;
    /*
     * Context quantum relative to other contexts of same priority in the same process
     * Minimum value supported by NPU is 1ms (10000 in 100ns units).
     */
    uint64_t context_quantum;
    /* Grace period when preempting context of the same priority within the same process */
    uint64_t grace_period_same_priority;
    /* Grace period when preempting context of a lower priority within the same process */
    uint64_t grace_period_lower_priority;
};

/*
 * @brief Register doorbell command structure.
 * This structure supports doorbell registration for both HW and OS scheduling.
 * Note: Queue base and size are added here so that the same structure can be used for
 * OS scheduling and HW scheduling. For OS scheduling, cmdq_id will be ignored
 * and cmdq_base and cmdq_size will be used. For HW scheduling, cmdq_base and cmdq_size will be
 * ignored and cmdq_id is used.
 * @see VPU_IPC_MSG_HWS_REGISTER_DB
 */
struct vpu_jsm_hws_register_db {
    /* Index of the doorbell to register. */
    uint32_t db_id;
    /* Host sub-stream ID for the context assigned to the doorbell. */
    uint32_t host_ssid;
    /* ID of the command queue associated with the doorbell. */
    uint64_t cmdq_id;
    /* Virtual address pointing to the start of command queue. */
    uint64_t cmdq_base;
    /* Size of the command queue in bytes. */
    uint64_t cmdq_size;
};

/*
 * @brief Structure to set another buffer to be used for scheduling-related logging.
 * The size of the logging buffer and the number of entries is defined as part of the
 * buffer itself as described next.
 * The log buffer received from the host is made up of;
 *   - header:     32 bytes in size, as shown in 'struct vpu_hws_log_buffer_header'.
 *                 The header contains the number of log entries in the buffer.
 *   - log entry:  0 to n-1, each log entry is 32 bytes in size, as shown in
 *                 'struct vpu_hws_log_buffer_entry'.
 *                 The entry contains the VPU timestamp, operation type and data.
 * The host should provide the notify index value of log buffer to VPU. This is a
 * value defined within the log buffer and when written to will generate the
 * scheduling log notification.
 * The host should set engine_idx and vpu_log_buffer_va to 0 to disable logging
 * for a particular engine.
 * VPU will handle one log buffer for each of supported engines.
 * VPU should allow the logging to consume one host_ssid.
 * @see VPU_IPC_MSG_HWS_SET_SCHEDULING_LOG
 * @see VPU_IPC_MSG_HWS_SET_SCHEDULING_LOG_RSP
 * @see VPU_IPC_MSG_HWS_SCHEDULING_LOG_NOTIFICATION
 */
struct vpu_ipc_msg_payload_hws_set_scheduling_log {
    /* Engine ordinal */
    uint32_t engine_idx;
    /* Host SSID */
    uint32_t host_ssid;
    /*
     * VPU log buffer virtual address.
     * Set to 0 to disable logging for this engine.
     */
    uint64_t vpu_log_buffer_va;
    /*
     * Notify index of log buffer. VPU_IPC_MSG_HWS_SCHEDULING_LOG_NOTIFICATION
     * is generated when an event log is written to this index.
     */
    uint64_t notify_index;
    /*
     * Field is now deprecated, will be removed when KMD is updated to support removal
     */
    uint32_t enable_extra_events;
    /* Zero Padding */
    uint32_t reserved_0;
};

/*
 * @brief The scheduling log notification is generated by VPU when it writes
 * an event into the log buffer at the notify_index. VPU notifies host with
 * VPU_IPC_MSG_HWS_SCHEDULING_LOG_NOTIFICATION. This is an asynchronous
 * message from VPU to host.
 * @see VPU_IPC_MSG_HWS_SCHEDULING_LOG_NOTIFICATION
 * @see VPU_IPC_MSG_HWS_SET_SCHEDULING_LOG
 */
struct vpu_ipc_msg_payload_hws_scheduling_log_notification {
    /* Engine ordinal */
    uint32_t engine_idx;
    /* Zero Padding */
    uint32_t reserved_0;
};

/*
 * @brief HWS suspend command queue request and done structure.
 * Host will request the suspend of contexts and VPU will;
 *   - Suspend all work on this context
 *   - Preempt any running work
 *   - Asynchronously perform the above and return success immediately once
 *     all items above are started successfully
 *   - Notify the host of completion of these operations via
 *     VPU_IPC_MSG_HWS_SUSPEND_CMDQ_DONE
 *   - Reject any other context operations on a context with an in-flight
 *     suspend request running
 * Same structure used when VPU notifies host of completion of a context suspend
 * request. The ids and suspend fence value reported in this command will match
 * the one in the request from the host to suspend the context. Once suspend is
 * complete, VPU will not access any data relating to this command queue until
 * it is resumed.
 * @see VPU_IPC_MSG_HWS_SUSPEND_CMDQ
 * @see VPU_IPC_MSG_HWS_SUSPEND_CMDQ_DONE
 */
struct vpu_ipc_msg_payload_hws_suspend_cmdq {
    /* Host SSID */
    uint32_t host_ssid;
    /* Zero Padding */
    uint32_t reserved_0;
    /* Command queue id */
    uint64_t cmdq_id;
    /*
     * Suspend fence value - reported by the VPU suspend context
     * completed once suspend is complete.
     */
    uint64_t suspend_fence_value;
};

/*
 * @brief HWS Resume command queue request / response structure.
 * Host will request the resume of a context;
 *  - VPU will resume all work on this context
 *  - Scheduler will allow this context to be scheduled
 * @see VPU_IPC_MSG_HWS_RESUME_CMDQ
 * @see VPU_IPC_MSG_HWS_RESUME_CMDQ_RSP
 */
struct vpu_ipc_msg_payload_hws_resume_cmdq {
    /* Host SSID */
    uint32_t host_ssid;
    /* Zero Padding */
    uint32_t reserved_0;
    /* Command queue id */
    uint64_t cmdq_id;
};

/*
 * @brief HWS Resume engine request / response structure.
 * After a HWS engine reset, all scheduling is stopped on VPU until a engine resume.
 * Host shall send this command to resume scheduling of any valid queue.
 * @see VPU_IPC_MSG_HWS_RESUME_ENGINE
 * @see VPU_IPC_MSG_HWS_RESUME_ENGINE_DONE
 */
struct vpu_ipc_msg_payload_hws_resume_engine {
    /* Engine to be resumed */
    uint32_t engine_idx;
    /* Reserved */
    uint32_t reserved_0;
};

/**
 * Payload for VPU_IPC_MSG_TRACE_SET_CONFIG[_RSP] and
 * VPU_IPC_MSG_TRACE_GET_CONFIG_RSP messages.
 *
 * The payload is interpreted differently depending on the type of message:
 *
 * - For VPU_IPC_MSG_TRACE_SET_CONFIG, the payload specifies the desired
 *   logging configuration to be set.
 *
 * - For VPU_IPC_MSG_TRACE_SET_CONFIG_RSP, the payload reports the logging
 *   configuration that was set after a VPU_IPC_MSG_TRACE_SET_CONFIG request.
 *   The host can compare this payload with the one it sent in the
 *   VPU_IPC_MSG_TRACE_SET_CONFIG request to check whether or not the
 *   configuration was set as desired.
 *
 * - VPU_IPC_MSG_TRACE_GET_CONFIG_RSP, the payload reports the current logging
 *   configuration.
 */
struct vpu_ipc_msg_payload_trace_config {
    /**
     * Logging level (currently set or to be set); see 'mvLog_t' enum for
     * acceptable values. The specified logging level applies to all
     * destinations and HW components
     */
    uint32_t trace_level;
    /**
     * Bitmask of logging destinations (currently enabled or to be enabled);
     * bitwise OR of values defined in logging_destination enum.
     */
    uint32_t trace_destination_mask;
    /**
     * Bitmask of loggable HW components (currently enabled or to be enabled);
     * bitwise OR of values defined in loggable_hw_component enum.
     */
    uint64_t trace_hw_component_mask;
    uint64_t reserved_0; /**< Reserved for future extensions. */
};

/**
 * Payload for VPU_IPC_MSG_TRACE_GET_CAPABILITY_RSP messages.
 */
struct vpu_ipc_msg_payload_trace_capability_rsp {
    uint32_t trace_destination_mask; /**< Bitmask of supported logging destinations. */
    uint32_t reserved_0;
    uint64_t trace_hw_component_mask; /**< Bitmask of supported loggable HW components. */
    uint64_t reserved_1;              /**< Reserved for future extensions. */
};

/**
 * Payload for VPU_IPC_MSG_TRACE_GET_NAME requests.
 */
struct vpu_ipc_msg_payload_trace_get_name {
    /**
     * The type of the entity to query name for; see logging_entity_type for
     * possible values.
     */
    uint32_t entity_type;
    uint32_t reserved_0;
    /**
     * The ID of the entity to query name for; possible values depends on the
     * entity type.
     */
    uint64_t entity_id;
};

/**
 * Payload for VPU_IPC_MSG_TRACE_GET_NAME_RSP responses.
 */
struct vpu_ipc_msg_payload_trace_get_name_rsp {
    /**
     * The type of the entity whose name was queried; see logging_entity_type
     * for possible values.
     */
    uint32_t entity_type;
    uint32_t reserved_0;
    /**
     * The ID of the entity whose name was queried; possible values depends on
     * the entity type.
     */
    uint64_t entity_id;
    /** Reserved for future extensions. */
    uint64_t reserved_1;
    /** The name of the entity. */
    char entity_name[VPU_TRACE_ENTITY_NAME_MAX_LEN];
};

/**
 * Data sent from the VPU to the host in all metric streamer response messages
 * and in asynchronous notification.
 * @see VPU_IPC_MSG_METRIC_STREAMER_START_DONE
 * @see VPU_IPC_MSG_METRIC_STREAMER_STOP_DONE
 * @see VPU_IPC_MSG_METRIC_STREAMER_UPDATE_DONE
 * @see VPU_IPC_MSG_METRIC_STREAMER_INFO_DONE
 * @see VPU_IPC_MSG_METRIC_STREAMER_NOTIFICATION
 */
struct vpu_jsm_metric_streamer_done {
    /** Metric group mask that identifies metric streamer instance. */
    uint64_t metric_group_mask;
    /**
     * Size in bytes of single sample - total size of all enabled counters.
     * Some VPU implementations may align sample_size to more than 8 bytes.
     */
    uint32_t sample_size;
    uint32_t reserved_0;
    /**
     * Number of samples collected since the metric streamer was started.
     * This will be 0 if the metric streamer was not started.
     */
    uint32_t samples_collected;
    /**
     * Number of samples dropped since the metric streamer was started. This
     * is incremented every time the metric streamer is not able to write
     * collected samples because the current buffer is full and there is no
     * next buffer to switch to.
     */
    uint32_t samples_dropped;
    /** Address of the buffer that contains the latest metric data. */
    uint64_t buffer_addr;
    /**
     * Number of bytes written into the metric data buffer. In response to the
     * VPU_IPC_MSG_METRIC_STREAMER_INFO request this field contains the size of
     * all group and counter descriptors. The size is updated even if the buffer
     * in the request was NULL or too small to hold descriptors of all counters
     */
    uint64_t bytes_written;
};
typedef struct vpu_jsm_metric_streamer_done vpu_jsm_metric_streamer_done_t;

/**
 * Metric group description placed in the metric buffer after successful completion
 * of the VPU_IPC_MSG_METRIC_STREAMER_INFO command. This is followed by one or more
 * @ref vpu_jsm_metric_counter_descriptor records.
 * @see VPU_IPC_MSG_METRIC_STREAMER_INFO
 */
struct vpu_jsm_metric_group_descriptor {
    /**
     * Offset to the next metric group (8-byte aligned). If this offset is 0 this
     * is the last descriptor. The value of metric_info_size must be greater than
     * or equal to sizeof(struct vpu_jsm_metric_group_descriptor) + name_string_size
     * + description_string_size and must be 8-byte aligned.
     */
    uint32_t next_metric_group_info_offset;
    /**
     * Offset to the first metric counter description record (8-byte aligned).
     * @see vpu_jsm_metric_counter_descriptor
     */
    uint32_t next_metric_counter_info_offset;
    /** Index of the group. This corresponds to bit index in metric_group_mask. */
    uint32_t group_id;
    /** Number of counters in the metric group. */
    uint32_t num_counters;
    /** Data size for all counters, must be a multiple of 8 bytes.*/
    uint32_t metric_group_data_size;
    /**
     * Metric group domain number. Cannot use multiple, simultaneous metric groups
     * from the same domain.
     */
    uint32_t domain;
    /**
     * Counter name string size. The string must include a null termination character.
     * The FW may use a fixed size name or send a different name for each counter.
     * If the VPU uses fixed size strings, all characters from the end of the name
     * to the of the fixed size character array must be zeroed.
     */
    uint32_t name_string_size;
    /** Counter description string size, @see name_string_size */
    uint32_t description_string_size;
    uint64_t reserved_0;
    /**
     * Right after this structure, the VPU writes name and description of
     * the metric group.
     */
};
typedef struct vpu_jsm_metric_group_descriptor vpu_jsm_metric_group_descriptor_t;

/**
 * Metric counter description, placed in the buffer after vpu_jsm_metric_group_descriptor.
 * @see VPU_IPC_MSG_METRIC_STREAMER_INFO
 */
struct vpu_jsm_metric_counter_descriptor {
    /**
     * Offset to the next counter in a group (8-byte aligned). If this offset is
     * 0 this is the last counter in the group.
     */
    uint32_t next_metric_counter_info_offset;
    /**
     * Offset to the counter data from the start of samples in this metric group.
     * Note that metric_data_offset % metric_data_size must be 0.
     */
    uint32_t metric_data_offset;
    /** Size of the metric counter data in bytes. */
    uint32_t metric_data_size;
    /** Metric type, see Level Zero API for definitions. */
    uint32_t tier;
    /** Metric type, see set_metric_type_t for definitions. */
    uint32_t metric_type;
    /** Metric type, see set_value_type_t for definitions. */
    uint32_t metric_value_type;
    /**
     * Counter name string size. The string must include a null termination character.
     * The FW may use a fixed size name or send a different name for each counter.
     * If the VPU uses fixed size strings, all characters from the end of the name
     * to the of the fixed size character array must be zeroed.
     */
    uint32_t name_string_size;
    /** Counter description string size, @see name_string_size */
    uint32_t description_string_size;
    /** Counter component name string size, @see name_string_size */
    uint32_t component_string_size;
    /** Counter string size, @see name_string_size */
    uint32_t units_string_size;
    uint64_t reserved_0;
    /**
     * Right after this structure, the VPU writes name, description
     * component and unit strings.
     */
};
typedef struct vpu_jsm_metric_counter_descriptor vpu_jsm_metric_counter_descriptor_t;

/**
 * Payload for VPU_IPC_MSG_DYNDBG_CONTROL requests.
 *
 * VPU_IPC_MSG_DYNDBG_CONTROL are used to control the VPU FW Dynamic Debug
 * feature, which allows developers to selectively enable / disable MVLOG_DEBUG
 * messages. This is equivalent to the Dynamic Debug functionality provided by
 * Linux
 * (https://www.kernel.org/doc/html/latest/admin-guide/dynamic-debug-howto.html)
 * The host can control Dynamic Debug behavior by sending dyndbg commands, which
 * have the same syntax as Linux
 * dyndbg commands.
 *
 * NOTE: in order for MVLOG_DEBUG messages to be actually printed, the host
 * still has to set the logging level to MVLOG_DEBUG, using the
 * VPU_IPC_MSG_TRACE_SET_CONFIG command.
 *
 * The host can see the current dynamic debug configuration by executing a
 * special 'show' command. The dyndbg configuration will be printed to the
 * configured logging destination using MVLOG_INFO logging level.
 */
struct vpu_ipc_msg_payload_dyndbg_control {
    /**
     * Dyndbg command (same format as Linux dyndbg); must be a NULL-terminated
     * string.
     */
    char dyndbg_cmd[VPU_DYNDBG_CMD_MAX_LEN];
};

/**
 * Payload for VPU_IPC_MSG_PWR_D0I3_ENTER
 *
 * This is a bi-directional payload.
 */
struct vpu_ipc_msg_payload_pwr_d0i3_enter {
    /**
     * 0: VPU_IPC_MSG_PWR_D0I3_ENTER_DONE is not sent to the host driver
     *    The driver will poll for D0i2 Idle state transitions.
     * 1: VPU_IPC_MSG_PWR_D0I3_ENTER_DONE is sent after VPU state save is complete
     */
    uint32_t send_response;
    uint32_t reserved_0;
};
typedef struct vpu_ipc_msg_payload_pwr_d0i3_enter vpu_ipc_msg_payload_pwr_d0i3_enter_t;

/**
 * Payload for VPU_IPC_MSG_DCT_ENABLE message.
 *
 * Default values for DCT active/inactive times are 5.3ms and 30ms respectively,
 * corresponding to a 85% duty cycle. This payload allows the host to tune these
 * values according to application requirements.
 */
struct vpu_ipc_msg_payload_pwr_dct_control {
    /** Duty cycle active time in microseconds */
    uint32_t dct_active_us;
    /** Duty cycle inactive time in microseconds */
    uint32_t dct_inactive_us;
};

/*
 * Payloads union, used to define complete message format.
 */
union vpu_ipc_msg_payload {
    struct vpu_ipc_msg_payload_engine_reset engine_reset;
    struct vpu_ipc_msg_payload_engine_preempt engine_preempt;
    struct vpu_ipc_msg_payload_register_db register_db;
    struct vpu_ipc_msg_payload_unregister_db unregister_db;
    struct vpu_ipc_msg_payload_query_engine_hb query_engine_hb;
    struct vpu_ipc_msg_payload_power_level power_level;
    struct vpu_jsm_metric_streamer_start metric_streamer_start;
    struct vpu_jsm_metric_streamer_stop metric_streamer_stop;
    struct vpu_jsm_metric_streamer_update metric_streamer_update;
    struct vpu_ipc_msg_payload_ssid_release ssid_release;
    struct vpu_jsm_hws_register_db hws_register_db;
    struct vpu_ipc_msg_payload_job_done job_done;
    struct vpu_ipc_msg_payload_native_fence_signalled native_fence_signalled;
    struct vpu_ipc_msg_payload_engine_reset_done engine_reset_done;
    struct vpu_ipc_msg_payload_engine_preempt_done engine_preempt_done;
    struct vpu_ipc_msg_payload_register_db_done register_db_done;
    struct vpu_ipc_msg_payload_unregister_db_done unregister_db_done;
    struct vpu_ipc_msg_payload_query_engine_hb_done query_engine_hb_done;
    struct vpu_ipc_msg_payload_get_power_level_count_done get_power_level_count_done;
    struct vpu_jsm_metric_streamer_done metric_streamer_done;
    struct vpu_ipc_msg_payload_trace_config trace_config;
    struct vpu_ipc_msg_payload_trace_capability_rsp trace_capability;
    struct vpu_ipc_msg_payload_trace_get_name trace_get_name;
    struct vpu_ipc_msg_payload_trace_get_name_rsp trace_get_name_rsp;
    struct vpu_ipc_msg_payload_dyndbg_control dyndbg_control;
    struct vpu_ipc_msg_payload_hws_priority_band_setup hws_priority_band_setup;
    struct vpu_ipc_msg_payload_hws_create_cmdq hws_create_cmdq;
    struct vpu_ipc_msg_payload_hws_create_cmdq_rsp hws_create_cmdq_rsp;
    struct vpu_ipc_msg_payload_hws_destroy_cmdq hws_destroy_cmdq;
    struct vpu_ipc_msg_payload_hws_set_context_sched_properties hws_set_context_sched_properties;
    struct vpu_ipc_msg_payload_hws_set_scheduling_log hws_set_scheduling_log;
    struct vpu_ipc_msg_payload_hws_scheduling_log_notification hws_scheduling_log_notification;
    struct vpu_ipc_msg_payload_hws_suspend_cmdq hws_suspend_cmdq;
    struct vpu_ipc_msg_payload_hws_resume_cmdq hws_resume_cmdq;
    struct vpu_ipc_msg_payload_hws_resume_engine hws_resume_engine;
    struct vpu_ipc_msg_payload_pwr_d0i3_enter pwr_d0i3_enter;
    struct vpu_ipc_msg_payload_pwr_dct_control pwr_dct_control;
};
typedef union vpu_ipc_msg_payload vpu_ipc_msg_payload_t;

/*
 * Host <-> LRT IPC message base structure.
 *
 * NOTE: All instances of this object must be aligned on a 64B boundary
 * to allow proper handling of VPU cache operations.
 */
struct vpu_ipc_msg {
    /* Reserved */
    uint64_t reserved_0;
    /* Message type, see vpu_ipc_msg_type enum. */
    uint32_t type;
    /* Buffer status, see vpu_ipc_msg_status enum. */
    uint32_t status;
    /*
     * Request ID, provided by the host in a request message and passed
     * back by VPU in the response message.
     */
    uint32_t request_id;
    /* Request return code set by the VPU, see VPU_JSM_STATUS_* defines. */
    uint32_t result;
    uint64_t reserved_1;
    /* Message payload depending on message type, see vpu_ipc_msg_payload union. */
    union vpu_ipc_msg_payload payload;
};
typedef struct vpu_ipc_msg vpu_ipc_msg_t;

#pragma pack(pop)

#endif

///@}
