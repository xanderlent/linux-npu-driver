/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "vpu_driver/source/command/vpu_command.hpp"
#include "vpu_driver/source/command/kmd_commit_command.hpp"

#include <uapi/drm/ivpu_drm.h>
#include <memory>

namespace VPU {

class VPUEventCommand : public VPUCommand {
  public:
    typedef uint64_t KMDEventDataType;
    enum State : KMDEventDataType {
        STATE_EVENT_INITIAL,
        STATE_DEVICE_RESET,
        STATE_HOST_RESET,
        STATE_DEVICE_SIGNAL,
        STATE_HOST_SIGNAL,
    };

    struct JsmEventData {
        uint64_t event;       /**< enum KMDEventDataType */
        uint64_t reserved[7]; /**< Unused */
    };
    static_assert(sizeof(JsmEventData) % 64 == 0, "JsmEventData is misaligned");

    /**
     * Returns size of the actual commit struct.
     */
    size_t getCommitSize() const override { return commitCmd.getKMDCommitSize(); }

    /**
     * Return the commiting struct data in byte stream.
     */
    const uint8_t *getCommitStream() const override { return commitCmd.getKMDCommitStream(); }

    /**
     * Return the command's type
     */
    vpu_cmd_type getCommandType() const override { return commitCmd.getKMDCommandType(); }

    /**
     * Set internal event buffer previously inserted with index
     */
    bool updateInternalEventOffsets(VPUDeviceContext *ctx, KMDEventDataType *eventHeapPtr);

    /**
     * Return index value of internal event
     */
    inline size_t getInternalEventIndex() const { return internalEventIndex; }

    static std::shared_ptr<VPUEventCommand> create(VPUDeviceContext *ctx,
                                                   const EngineSupport engType,
                                                   const vpu_cmd_type cmdType,
                                                   KMDEventDataType *eventHeapPtr,
                                                   const KMDEventDataType eventState);

    VPUEventCommand(VPUDeviceContext *ctx,
                    const EngineSupport engType,
                    const vpu_cmd_type cmdType,
                    KMDEventDataType *eventHeapPtr,
                    const KMDEventDataType eventState);

    VPUEventCommand(const EngineSupport engType,
                    const vpu_cmd_type cmdType,
                    const KMDEventDataType eventState,
                    uint8_t intEventIndex = 0);

    /**
     * KMD submitting fence command.
     */
    KMDCommitCommand<vpu_cmd_fence_t> commitCmd;

  private:
    static const char *getEventCommandStr(const vpu_cmd_type cmdType,
                                          const KMDEventDataType eventState);
    size_t internalEventIndex = 0u;
};

class VPUEventResetCommand : public VPUEventCommand {
  public:
    static std::shared_ptr<VPUEventCommand> create(VPUDeviceContext *ctx,
                                                   KMDEventDataType *eventHeapPtr) {
        return VPUEventCommand::create(ctx,
                                       EngineSupport::Backward,
                                       VPU_CMD_FENCE_SIGNAL,
                                       eventHeapPtr,
                                       VPUEventCommand::STATE_DEVICE_RESET);
    }

    static std::shared_ptr<VPUEventCommand> create(uint8_t intEventIndex) {
        return std::make_shared<VPUEventCommand>(EngineSupport::Backward,
                                                 VPU_CMD_FENCE_SIGNAL,
                                                 VPUEventCommand::STATE_DEVICE_RESET,
                                                 intEventIndex);
    }
};

class VPUEventSignalCommand : public VPUEventCommand {
  public:
    static std::shared_ptr<VPUEventCommand> create(VPUDeviceContext *ctx,
                                                   KMDEventDataType *eventHeapPtr) {
        return VPUEventCommand::create(ctx,
                                       EngineSupport::Backward,
                                       VPU_CMD_FENCE_SIGNAL,
                                       eventHeapPtr,
                                       VPUEventCommand::STATE_DEVICE_SIGNAL);
    }

    static std::shared_ptr<VPUEventCommand> create(uint8_t intEventIndex) {
        return std::make_shared<VPUEventCommand>(EngineSupport::Backward,
                                                 VPU_CMD_FENCE_SIGNAL,
                                                 VPUEventCommand::STATE_DEVICE_SIGNAL,
                                                 intEventIndex);
    }
};

class VPUEventWaitCommand : public VPUEventCommand {
  public:
    static std::shared_ptr<VPUEventCommand> create(VPUDeviceContext *ctx,
                                                   KMDEventDataType *eventHeapPtr) {
        return VPUEventCommand::create(ctx,
                                       EngineSupport::Forward,
                                       VPU_CMD_FENCE_WAIT,
                                       eventHeapPtr,
                                       VPUEventCommand::STATE_DEVICE_SIGNAL);
    }

    static std::shared_ptr<VPUEventCommand> create(uint8_t intEventIndex) {
        return std::make_shared<VPUEventCommand>(EngineSupport::Forward,
                                                 VPU_CMD_FENCE_WAIT,
                                                 VPUEventCommand::STATE_DEVICE_SIGNAL,
                                                 intEventIndex);
    }
};

} // namespace VPU
