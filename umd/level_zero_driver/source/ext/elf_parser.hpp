/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <cstdint>

#include "interface_parser.hpp"
#include "level_zero/ze_graph_ext.h"
#include "vpu_driver/source/command/vpu_command.hpp"
#include "vpux_elf/utils/version.hpp"

#include <level_zero/ze_api.h>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <vpux_elf/accessor.hpp>
#include <vpux_headers/buffer_manager.hpp>
#include <vpux_hpi.hpp>

namespace VPU {
class VPUBufferObject;
class VPUDeviceContext;
class VPUInferenceExecute;
} // namespace VPU

namespace L0 {

class BlobContainer;

class ElfParser : public IParser, public std::enable_shared_from_this<ElfParser> {
  public:
    ElfParser(VPU::VPUDeviceContext *ctx,
              std::unique_ptr<elf::BufferManager> manager,
              std::unique_ptr<elf::AccessManager> access,
              std::shared_ptr<elf::HostParsedInference> loader);

    static bool checkMagic(const std::unique_ptr<BlobContainer> &blob);
    static std::unique_ptr<ElfParser> getElfParser(VPU::VPUDeviceContext *ctx,
                                                   const std::unique_ptr<BlobContainer> &blob,
                                                   std::string &logBuffer);
    static elf::VersionsProvider getElfVer(uint32_t deviceId);

    bool getArgumentProperties(std::vector<ze_graph_argument_properties_3_t> &props) const;
    bool getArgumentMetadata(std::vector<ze_graph_argument_metadata_t> &args) const;
    bool getProfilingSize(uint32_t &size) const;

    std::shared_ptr<VPU::VPUInferenceExecute>
    createInferenceExecuteCommand(const std::vector<std::pair<const void *, uint32_t>> &inputPtrs,
                                  const std::vector<std::pair<const void *, uint32_t>> &outputPtrs,
                                  const std::pair<void *, uint32_t> &profilingPtr);

    ze_result_t parse(std::vector<ze_graph_argument_properties_3_t> &argumentProperties,
                      std::vector<ze_graph_argument_metadata_t> &argumentMetadata,
                      uint32_t &profilingOutputSize) override;

    ze_result_t initialize() override;

    std::shared_ptr<VPU::VPUCommand> allocateInitCommand(VPU::VPUDeviceContext *ctx) override;

    std::shared_ptr<VPU::VPUCommand>
    allocateExecuteCommand(VPU::VPUDeviceContext *ctx,
                           const std::vector<std::pair<const void *, uint32_t>> &inputArgs,
                           const std::vector<std::pair<const void *, uint32_t>> &outputArgs,
                           const std::pair<void *, uint32_t> &profilingPtr) override;

    bool applyInputOutputs(std::shared_ptr<elf::HostParsedInference> &hpi,
                           const std::vector<std::pair<const void *, uint32_t>> &inputs,
                           const std::vector<std::pair<const void *, uint32_t>> &outputs,
                           const std::pair<const void *, uint32_t> &profilingPtr,
                           std::vector<std::shared_ptr<VPU::VPUBufferObject>> &bos);
    std::shared_ptr<VPU::VPUBufferObject> findBuffer(const void *ptr);

  private:
    VPU::VPUDeviceContext *ctx;
    std::unique_ptr<elf::BufferManager> bufferManager;
    std::unique_ptr<elf::AccessManager> accessManager;
    std::shared_ptr<elf::HostParsedInference> hpi;
    bool needCopy = false;
    bool needLoad = true;
    std::mutex loadMutex;
};

template <class T>
inline T toVersion(uint64_t ver) {
    uint32_t major = ver >> 16 & UINT16_MAX;
    uint32_t minor = ver & UINT16_MAX;

    if constexpr (std::is_same<T, elf::Version>::value)
        return elf::Version(major, minor, 0);

    if constexpr (std::is_same<T, ze_graph_version_info_t>::value)
        return ze_graph_version_info_t{major, minor, 0};
}

} // namespace L0
