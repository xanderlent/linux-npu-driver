/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "level_zero_driver/core/source/device/device.hpp"
#include "level_zero_driver/tools/source/metrics/metric.hpp"
#include "level_zero_driver/tools/source/metrics/metric_query.hpp"
#include "vpu_driver/source/device/device_info.hpp"

namespace L0 {
namespace ult {

class MockMetricDevice : public L0::Device {
  public:
    MockMetricDevice(DriverHandle *driverHandle, VPU::VPUDevice *vpuDevice)
        : L0::Device(driverHandle, vpuDevice) {}
};

class MockMetricContext : public L0::MetricContext {
  public:
    MockMetricContext(Device *deviceInput)
        : L0::MetricContext(deviceInput) {}
    size_t getActivatedMetricGroupsSize() const { return activatedMetricGroups.size(); }
};

class MockMetricQuery : public L0::MetricQuery {
  public:
    MockMetricQuery(MetricGroup &metricGroupInput,
                    VPU::VPUDeviceContext *ctx,
                    MetricQueryPool *poolInput,
                    uint32_t indexInput,
                    uint64_t *queryPtrInput)
        : L0::MetricQuery(metricGroupInput, ctx, poolInput, indexInput, queryPtrInput) {}
    void injectDummyData();
};

} // namespace ult
} // namespace L0