// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file HelloWorld_main.cpp
 *
 */

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastrtps;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds::rtps;

class PartListener : public eprosima::fastdds::dds::DomainParticipantListener
{
public:
    PartListener()
        : discovered_count_(0)
    {
    }
    virtual void on_participant_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;
    int discovered_count_;
};

void PartListener::on_participant_discovery(
        DomainParticipant*,
        ParticipantDiscoveryInfo&& info)
{
    if (info.status == ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        ++discovered_count_;
        std::cout << info.info.m_participantName << " discovered." << discovered_count_ << std::endl;
    }
    else if (info.status == ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
    {
        --discovered_count_;
        std::cout << info.info.m_participantName << " dropped." << discovered_count_ << std::endl;
    }
    else if (info.status == ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
    {
        --discovered_count_;
        std::cout << info.info.m_participantName << " removed." << discovered_count_ << std::endl;
    }
}

int main(
        int argc,
        char** argv)
{
    // PARTICIPANT QoS
    DomainParticipantQos pqos;
    pqos.name(std::string("Participant_Discovery_Srv"));
    int leaseDuration = 8;
    char* leaseDuration_env = getenv("leaseDuration");
    if (leaseDuration_env) {
        leaseDuration = atoi(leaseDuration_env);
    }
    int leaseDuration_announcementperiod = 2;
    char* leaseDuration_announcementperiod_env = getenv("leaseDuration_announcementperiod");
    if (leaseDuration_announcementperiod_env) {
        leaseDuration_announcementperiod = atoi(leaseDuration_announcementperiod_env);
    }
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::Duration_t(leaseDuration, 0);
    pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = eprosima::fastrtps::Duration_t(leaseDuration_announcementperiod, 0);
    pqos.wire_protocol().builtin.mutation_tries = 8192;
    auto transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
    transport->TTL = 64;
    pqos.transport().user_transports.push_back(transport);
    pqos.transport().use_builtin_transports = false;

    // Set participant as SERVER
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::SERVER;
    // Set SERVER's GUID prefix
    std::istringstream("44.53.00.5f.45.50.52.4f.53.49.4d.41") >> pqos.wire_protocol().prefix;
    // Set SERVER's listening locator for PDP
    Locator_t locator;
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 11811;
    pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);

    RemoteServerAttributes remote_server_att;
    remote_server_att.ReadguidPrefix("44.53.00.5f.45.50.52.4f.53.49.4d.41");

    // Set remote SERVER's listening locator for PDP
    Locator_t remote_locator;
    IPLocator::setIPv4(remote_locator, 127, 0, 0, 1);
    remote_locator.port = 11812;
    remote_server_att.metatrafficUnicastLocatorList.push_back(remote_locator);

    // Add remote SERVER to SERVER's list of SERVERs
    pqos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(remote_server_att);

    // LOCATOR
    #if 0
    eprosima::fastrtps::rtps::Locator_t ddsLocator;
    eprosima::fastrtps::rtps::IPLocator::setIPv4(ddsLocator, 239, 255, 0, 2);
    ddsLocator.port = 7400;
    pqos.wire_protocol().builtin.metatrafficMulticastLocatorList.push_back(ddsLocator);

    eprosima::fastrtps::rtps::IPLocator::setIPv4(ddsLocator, 0, 0, 0, 0);
    ddsLocator.port = 8400 + index * 2;
    pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(ddsLocator);
    ddsLocator.port = 8401 + index * 2;
    pqos.wire_protocol().default_unicast_locator_list.push_back(ddsLocator);
    #endif
    
    // AUTH AND ENCRYPTION
    pqos.properties().properties().emplace_back("dds.sec.auth.plugin",
            "builtin.PKI-DH");
    pqos.properties().properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://certs/maincacert.pem");
    pqos.properties().properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://certs/mainsubcert.pem");
    pqos.properties().properties().emplace_back("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://certs/mainsubkey.pem");
    pqos.properties().properties().emplace_back("dds.sec.access.plugin",
            "builtin.Access-Permissions");
    pqos.properties().properties().emplace_back("dds.sec.access.builtin.Access-Permissions.permissions_ca",
            "file://certs/maincacert.pem");
    pqos.properties().properties().emplace_back("dds.sec.access.builtin.Access-Permissions.governance",
            "file://certs/governance.smime");
    pqos.properties().properties().emplace_back("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://certs/permissions.smime");
    pqos.properties().properties().emplace_back("dds.sec.crypto.plugin",
            "builtin.AES-GCM-GMAC");

    // CREATE THE PARTICIPANT
    eprosima::fastdds::dds::StatusMask callback_mask = eprosima::fastdds::dds::StatusMask::none();
    PartListener part_listener_;
    eprosima::fastdds::dds::DomainParticipant* participant_ = DomainParticipantFactory::get_instance()->create_participant(2, pqos, &part_listener_, callback_mask);

    if (participant_ == nullptr)
    {
        return -1;
    }

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
    Domain::stopAll();
    Log::Reset();
    return 0;

    return 0;
}
