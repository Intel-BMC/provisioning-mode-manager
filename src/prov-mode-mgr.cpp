/*
// Copyright (c) 2019 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include "prov-mode-mgr.hpp"
#include <phosphor-logging/log.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <xyz/openbmc_project/Common/error.hpp>

ProvModeMgr::ProvModeMgr(
    boost::asio::io_service& ioService, sdbusplus::asio::object_server& srv,
    std::shared_ptr<sdbusplus::asio::connection>& connection) :
    io(ioService),
    server(srv), conn(connection),
    provMode(sdbusplus::xyz::openbmc_project::Control::Security::server::
                 RestrictionMode::Modes::Provisioning)
{
    namespace secCtrl =
        sdbusplus::xyz::openbmc_project::Control::Security::server;
    conn->async_method_call(
        [this](boost::system::error_code ec, const std::string& modeStr) {
            if (ec)
            {
                phosphor::logging::log<phosphor::logging::level::ERR>(
                    "Error in querying provision mode",
                    phosphor::logging::entry("MSG=%s", ec.message().c_str()));
                phosphor::logging::elog<sdbusplus::xyz::openbmc_project::
                                            Common::Error::InternalFailure>();
            }
            if (modeStr.empty())
            {
                updateProvModeProperty(provMode);
            }
            else
            {
                provMode = static_cast<secCtrl::RestrictionMode::Modes>(
                    std::stoi(modeStr, nullptr, 10));
            }
            init();
        },
        uBootEnvMgrService, uBootEnvMgrPath, uBootEnvMgrIntf,
        uBootEnvMgrReadMethod, uBootEnvProvision);
}

void ProvModeMgr::updateProvModeProperty(
    sdbusplus::xyz::openbmc_project::Control::Security::server::
        RestrictionMode::Modes mode)
{
    conn->async_method_call(
        [this](boost::system::error_code ec) {
            if (ec)
            {
                phosphor::logging::log<phosphor::logging::level::ERR>(
                    "RestrictionMode set-property failed",
                    phosphor::logging::entry("MSG=%s", ec.message().c_str()));
                phosphor::logging::elog<sdbusplus::xyz::openbmc_project::
                                            Common::Error::InternalFailure>();
            }
        },
        uBootEnvMgrService, uBootEnvMgrPath, uBootEnvMgrIntf,
        uBootEnvMgrWriteMethod, uBootEnvProvision,
        std::to_string(static_cast<uint8_t>(mode)));
}

void ProvModeMgr::init()
{
    namespace secCtrl =
        sdbusplus::xyz::openbmc_project::Control::Security::server;
    iface = server.add_interface(provModePath, provModeIntf);
    iface->register_property(
        "RestrictionMode",
        sdbusplus::xyz::openbmc_project::Control::Security::server::
            convertForMessage(provMode),
        [this](const std::string& req, std::string& propertyValue) {
            try
            {
                if (req != propertyValue)
                {
                    propertyValue = req;
                    secCtrl::RestrictionMode::Modes mode =
                        secCtrl::RestrictionMode::convertModesFromString(req);
                    provMode = mode;
                    updateProvModeProperty(mode);
                    return 1;
                }
            }
            catch (const std::exception& e)
            {
                phosphor::logging::log<phosphor::logging::level::ERR>(
                    "RestrictionMode set-property failed",
                    phosphor::logging::entry("Mode=%s", req.c_str()),
                    phosphor::logging::entry("EXCEPTION:%s", e.what()));
                phosphor::logging::elog<sdbusplus::xyz::openbmc_project::
                                            Common::Error::InternalFailure>();
            }
            return 0;
        },
        [this](const std::string& propertyValue) {
            return sdbusplus::xyz::openbmc_project::Control::Security::server::
                convertForMessage(provMode);
        });
    iface->initialize(true);
}
