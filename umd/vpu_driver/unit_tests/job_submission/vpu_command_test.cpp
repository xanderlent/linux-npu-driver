/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stdint.h>

#include "api/vpu_jsm_job_cmd_api.h"
#include "gtest/gtest.h"
#include "vpu_driver/source/command/vpu_barrier_command.hpp"
#include "vpu_driver/source/command/vpu_command.hpp"
#include "vpu_driver/source/command/vpu_copy_command.hpp"
#include "vpu_driver/source/command/vpu_event_command.hpp"
#include "vpu_driver/source/command/vpu_query_command.hpp"
#include "vpu_driver/source/command/vpu_ts_command.hpp"
#include "vpu_driver/source/memory/vpu_buffer_object.hpp"
#include "vpu_driver/unit_tests/mocks/mock_os_interface_imp.hpp"
#include "vpu_driver/unit_tests/mocks/mock_vpu_device.hpp"

#include <memory>
#include <string.h>
#include <string>
#include <vector>

using namespace VPU;

struct VPUCommandTest : public ::testing::Test {
    void TearDown() override {
        if (ctx) {
            ASSERT_EQ(ctx->getBuffersCount(), 0u);
        }
    }

    MockOsInterfaceImp osInfc;
    std::unique_ptr<MockVPUDevice> vpuDevice = MockVPUDevice::createWithDefaultHardwareInfo(osInfc);
    std::unique_ptr<MockVPUDeviceContext> deviceContext = vpuDevice->createMockDeviceContext();
    MockVPUDeviceContext *ctx = deviceContext.get();
};

TEST_F(VPUCommandTest, timestampCommandShouldReturnExpectedProperties) {
    auto mem = ctx->createSharedMemAlloc(sizeof(uint64_t));

    // Time stamp command.
    VPUTimeStampCommand tsCmd(ctx->getFwTimestampType(), mem->getVPUAddr(), mem);
    EXPECT_EQ(VPU_CMD_TIMESTAMP, tsCmd.getCommandType());
    EXPECT_EQ(sizeof(vpu_cmd_timestamp_t), tsCmd.getCommitSize());

    // Compare associated pointers
    EXPECT_EQ(tsCmd.getAssociateBufferObjects().size(), 1u);
    EXPECT_EQ(tsCmd.getAssociateBufferObjects().at(0).get(), mem.get());

    // Compare command stream return value in byte wise.
    vpu_cmd_timestamp_t expKMDTsCmd = {};
    expKMDTsCmd.header = {VPU_CMD_TIMESTAMP, sizeof(vpu_cmd_timestamp_t)};
    expKMDTsCmd.timestamp_address = mem->getVPUAddr();
    expKMDTsCmd.type = VPU_TIME_RAW;

    EXPECT_EQ(sizeof(vpu_cmd_timestamp_t), tsCmd.getCommitSize());
    EXPECT_EQ(memcmp(&expKMDTsCmd, tsCmd.getCommitStream(), sizeof(vpu_cmd_timestamp_t)), 0);

    EXPECT_TRUE(ctx->freeMemAlloc(mem->getBasePointer()));
}

TEST_F(VPUCommandTest, copyCommandShouldReturnExpectedProperties) {
    auto srcBo = ctx->createSharedMemAlloc(sizeof(uint64_t));
    auto dstBo = ctx->createSharedMemAlloc(sizeof(uint64_t));

    // Copy command.
    std::shared_ptr<VPUCommand> copyCmd = VPUCopyCommand::create(ctx,
                                                                 srcBo->getBasePointer(),
                                                                 srcBo,
                                                                 dstBo->getBasePointer(),
                                                                 dstBo,
                                                                 sizeof(uint64_t));
    ASSERT_NE(copyCmd, nullptr);

    EXPECT_EQ(VPU_CMD_COPY_LOCAL_TO_LOCAL, copyCmd->getCommandType());
    EXPECT_EQ(sizeof(vpu_cmd_copy_buffer_t), copyCmd->getCommitSize());

    // Compare buffer handle
    auto copyCmdAssocVec = copyCmd->getAssociateBufferObjects();
    ASSERT_EQ(copyCmdAssocVec.size(), 2u);
    VPUBufferObject *bo = srcBo.get();
    ASSERT_NE(bo, nullptr);
    EXPECT_EQ(copyCmdAssocVec.at(0).get(), bo);
    bo = dstBo.get();
    ASSERT_NE(bo, nullptr);
    EXPECT_EQ(copyCmdAssocVec.at(1).get(), bo);

    // Compare command stream return value in byte wise.
    vpu_cmd_copy_buffer_t
        expKMDCopyCmd{{VPU_CMD_COPY_LOCAL_TO_LOCAL, sizeof(vpu_cmd_copy_buffer_t)}, 0u, 0u, 1u, 0u};
    uint8_t *exp = reinterpret_cast<uint8_t *>(&expKMDCopyCmd);

    EXPECT_EQ(memcmp(exp, copyCmd->getCommitStream(), sizeof(vpu_cmd_copy_buffer_t)), 0);

    EXPECT_TRUE(ctx->freeMemAlloc(srcBo->getBasePointer()));
    EXPECT_TRUE(ctx->freeMemAlloc(dstBo->getBasePointer()));
}

TEST_F(VPUCommandTest, barrierCommandShouldReturnExpectedProperties) {
    std::shared_ptr<VPUCommand> barrierCmd = VPUBarrierCommand::create();
    ASSERT_NE(barrierCmd, nullptr);

    EXPECT_EQ(VPU_CMD_BARRIER, barrierCmd->getCommandType());
    EXPECT_EQ(sizeof(vpu_cmd_barrier_t), barrierCmd->getCommitSize());

    // Compare command stream return value in byte wise.
    vpu_cmd_barrier_t expKMDBarrierCmd{{VPU_CMD_BARRIER, sizeof(vpu_cmd_barrier_t)}, 0};
    uint8_t *exp = reinterpret_cast<uint8_t *>(&expKMDBarrierCmd);

    EXPECT_EQ(sizeof(vpu_cmd_barrier_t), barrierCmd->getCommitSize());
    EXPECT_EQ(memcmp(exp, barrierCmd->getCommitStream(), sizeof(vpu_cmd_barrier)), 0);
}

TEST_F(VPUCommandTest, queryBeginShouldReturnExpectedProperties) {
    auto mem = ctx->createSharedMemAlloc(sizeof(uint64_t));

    std::shared_ptr<VPUCommand> queryBeginCmd =
        VPUQueryBeginCommand::create(0u, mem->getBasePointer(), mem);
    ASSERT_NE(queryBeginCmd, nullptr);
    EXPECT_EQ(VPU_CMD_METRIC_QUERY_BEGIN, queryBeginCmd->getCommandType());
    EXPECT_EQ(sizeof(vpu_cmd_metric_query_t), queryBeginCmd->getCommitSize());

    // Compare command stream return value in byte wise.
    vpu_cmd_metric_query_t expKMDQBeginCmd{
        {VPU_CMD_METRIC_QUERY_BEGIN, sizeof(vpu_cmd_metric_query_t)},
        0u,
        mem->getVPUAddr()};
    uint8_t *exp = reinterpret_cast<uint8_t *>(&expKMDQBeginCmd);

    EXPECT_EQ(memcmp(exp, queryBeginCmd->getCommitStream(), sizeof(vpu_cmd_metric_query_t)), 0);

    EXPECT_TRUE(ctx->freeMemAlloc(mem->getBasePointer()));
}

TEST_F(VPUCommandTest, queryEndShouldReturnExpectedProperties) {
    auto mem = ctx->createSharedMemAlloc(sizeof(uint64_t));

    std::shared_ptr<VPUCommand> queryEndCmd =
        VPUQueryEndCommand::create(0u, mem->getBasePointer(), mem);
    ASSERT_NE(queryEndCmd, nullptr);

    EXPECT_EQ(VPU_CMD_METRIC_QUERY_END, queryEndCmd->getCommandType());
    EXPECT_EQ(sizeof(vpu_cmd_metric_query_t), queryEndCmd->getCommitSize());

    // Compare command stream return value in byte wise.
    vpu_cmd_metric_query_t expKMDQEndCmd{{VPU_CMD_METRIC_QUERY_END, sizeof(vpu_cmd_metric_query_t)},
                                         0u,
                                         mem->getVPUAddr()};
    uint8_t *exp = reinterpret_cast<uint8_t *>(&expKMDQEndCmd);

    EXPECT_EQ(memcmp(exp, queryEndCmd->getCommitStream(), sizeof(vpu_cmd_metric_query_t)), 0);

    EXPECT_TRUE(ctx->freeMemAlloc(mem->getBasePointer()));
}

struct VPUEventCommandTest : public VPUCommandTest {
    void SetUp() {
        VPUCommandTest::SetUp();

        eventBo = ctx->createUntrackedBufferObject(4096, VPUBufferObject::Type::CachedFw);
        ASSERT_TRUE(eventBo);

        eventHeapPtr = reinterpret_cast<decltype(eventHeapPtr)>(eventBo->getBasePointer());
    }

    void TearDown() {
        eventBo.reset();
        VPUCommandTest::TearDown();
    }

    std::shared_ptr<VPUBufferObject> eventBo = nullptr;
    VPUEventCommand::KMDEventDataType *eventHeapPtr = nullptr;
    vpu_cmd_buffer_header_t cmdBufferHeader = {};
};

TEST_F(VPUEventCommandTest, eventWaitCommandsShouldReturnExpectedProperties) {
    // 0 offset, Event wait command.
    std::shared_ptr<VPUCommand> waitCmd = VPUEventWaitCommand::create(eventHeapPtr, eventBo);

    ASSERT_NE(waitCmd, nullptr);

    // Check expected command contents.
    EXPECT_EQ(VPU_CMD_FENCE_WAIT, waitCmd->getCommandType());
    EXPECT_EQ(sizeof(vpu_cmd_fence_t), waitCmd->getCommitSize());

    const vpu_cmd_fence_t *actual =
        reinterpret_cast<const vpu_cmd_fence_t *>(waitCmd->getCommitStream());
    EXPECT_EQ(VPU_CMD_FENCE_WAIT, actual->header.type);
    EXPECT_EQ(sizeof(vpu_cmd_fence_t), actual->header.size);
    EXPECT_NE(0u, actual->offset);
    EXPECT_EQ(VPUEventCommand::STATE_WAIT, actual->value);
    uint64_t offset1 = actual->offset;

    // 64bits offsetted event wait command.
    VPUEventCommand::KMDEventDataType *offsetEventHeapPtr = eventHeapPtr + 1;

    std::shared_ptr<VPUCommand> waitCmd2 = VPUEventWaitCommand::create(offsetEventHeapPtr, eventBo);

    ASSERT_NE(waitCmd2, nullptr);

    actual = reinterpret_cast<const vpu_cmd_fence_t *>(waitCmd2->getCommitStream());
    EXPECT_EQ(VPU_CMD_FENCE_WAIT, actual->header.type);
    EXPECT_EQ(sizeof(vpu_cmd_fence_t), actual->header.size);
    EXPECT_EQ(actual->offset, offset1 + 8ULL);
    EXPECT_EQ(VPUEventCommand::STATE_WAIT, actual->value);
}

TEST_F(VPUEventCommandTest, eventSignalCommandsShouldReturnExpectedProperties) {
    std::shared_ptr<VPUCommand> signalCmd = VPUEventSignalCommand::create(eventHeapPtr, eventBo);
    ASSERT_NE(signalCmd, nullptr);

    // Check expected command contents.
    EXPECT_EQ(VPU_CMD_FENCE_SIGNAL, signalCmd->getCommandType());
    EXPECT_EQ(sizeof(vpu_cmd_fence_t), signalCmd->getCommitSize());
    EXPECT_TRUE(signalCmd->isSynchronizeCommand());

    // Compare command stream return value in byte wise.
    vpu_cmd_fence_t expKMDSignalCmd = {};
    expKMDSignalCmd.header = {VPU_CMD_FENCE_SIGNAL, sizeof(vpu_cmd_fence_t)};
    expKMDSignalCmd.offset = reinterpret_cast<uint64_t>(eventBo->getVPUAddr(eventHeapPtr));
    expKMDSignalCmd.value = VPUEventCommand::STATE_DEVICE_SIGNAL;

    EXPECT_EQ(memcmp(&expKMDSignalCmd, signalCmd->getCommitStream(), sizeof(vpu_cmd_fence_t)), 0);

    // 64bits offsetted event signal command.
    VPUEventCommand::KMDEventDataType *offsetEventHeapPtr = eventHeapPtr + 1;
    std::shared_ptr<VPUCommand> signalCmd2 =
        VPUEventSignalCommand::create(offsetEventHeapPtr, eventBo);
    ASSERT_NE(signalCmd2, nullptr);
    EXPECT_EQ(sizeof(vpu_cmd_fence_t), signalCmd2->getCommitSize());

    expKMDSignalCmd.offset = reinterpret_cast<uint64_t>(eventBo->getVPUAddr(eventHeapPtr)) + 8ULL;
    EXPECT_EQ(memcmp(&expKMDSignalCmd, signalCmd2->getCommitStream(), sizeof(vpu_cmd_fence_t)), 0);
}

TEST_F(VPUEventCommandTest, eventResetCommandsShouldReturnExpectedProperties) {
    std::shared_ptr<VPUCommand> resetCmd = VPUEventResetCommand::create(eventHeapPtr, eventBo);

    ASSERT_NE(resetCmd, nullptr);

    // Check expected command contents.
    EXPECT_EQ(VPU_CMD_FENCE_SIGNAL, resetCmd->getCommandType());
    EXPECT_EQ(sizeof(vpu_cmd_fence_t), resetCmd->getCommitSize());

    // Compare command stream return value in byte wise.
    vpu_cmd_fence_t expKMDResetCmd = {};
    expKMDResetCmd.header = {VPU_CMD_FENCE_SIGNAL, sizeof(vpu_cmd_fence_t)};
    expKMDResetCmd.offset = reinterpret_cast<uint64_t>(eventBo->getVPUAddr(eventHeapPtr));
    expKMDResetCmd.value = VPUEventCommand::State::STATE_DEVICE_RESET;

    EXPECT_EQ(sizeof(vpu_cmd_fence_t), resetCmd->getCommitSize());
    EXPECT_EQ(memcmp(&expKMDResetCmd, resetCmd->getCommitStream(), sizeof(vpu_cmd_fence_t)), 0);
}
