/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "webrtc-stack.h"
#include <lib/support/logging/CHIPLogging.h>
#include <rtc/rtc.hpp>

namespace {

// Constants
constexpr int kVideoH264PayloadType = 96;
constexpr int kVideoBitRate         = 3000;

class LibDataChannelTrack : public WebRTCTrack
{
public:
    LibDataChannelTrack(std::shared_ptr<rtc::Track> track) : mTrack(track) {}

    void SendData(const char * data, size_t size) override
    {
        if (mTrack)
        {
            mTrack->send(reinterpret_cast<const std::byte *>(data), size);
        }
    }

    bool IsReady() override { return mTrack != nullptr && mTrack->isOpen(); }

    std::string GetType() override
    {
        if (mTrack)
        {
            auto description = mTrack->description();
            return std::string(description.type());
        }
        return "";
    }

private:
    std::shared_ptr<rtc::Track> mTrack;
};

class LibDataChannelPeerConnection : public WebRTCPeerConnection
{
public:
    LibDataChannelPeerConnection()
    {
        rtc::Configuration config;
        mPeerConnection = std::make_shared<rtc::PeerConnection>(config);
    }

    void SetCallbacks(OnLocalDescriptionCallback onLocalDescription, OnICECandidateCallback onICECandidate,
                      OnConnectionStateCallback onConnectionState, OnTrackCallback onTrack) override
    {
        mPeerConnection->onLocalDescription([onLocalDescription](rtc::Description desc) {
            onLocalDescription(std::string(desc), std::string(desc.type()));
        });

        mPeerConnection->onLocalCandidate([onICECandidate](rtc::Candidate candidate) { onICECandidate(std::string(candidate)); });

        mPeerConnection->onStateChange([onConnectionState](rtc::PeerConnection::State state) {
            ChipLogProgress(Camera, "[PeerConnection State: %d]", static_cast<int>(state));
            if (state == rtc::PeerConnection::State::Connected)
            {
                onConnectionState(true);
            }
            else if (state == rtc::PeerConnection::State::Failed || state == rtc::PeerConnection::State::Closed ||
                     state == rtc::PeerConnection::State::Disconnected)
            {
                onConnectionState(false);
            }
        });

        mPeerConnection->onTrack([onTrack](std::shared_ptr<rtc::Track> track) {
            onTrack(std::make_shared<LibDataChannelTrack>(track));
        });
    }

    void Close() override
    {
        if (mPeerConnection)
        {
            mPeerConnection->close();
        }
    }

    void CreateOffer() override { mPeerConnection->setLocalDescription(); }

    void CreateAnswer() override { mPeerConnection->createAnswer(); }

    void SetRemoteDescription(const std::string & sdp, const std::string & type) override
    {
        rtc::Description::Type rtcType =
            (type == "offer") ? rtc::Description::Type::Offer : rtc::Description::Type::Answer;
        mPeerConnection->setRemoteDescription(rtc::Description(sdp, rtcType));
    }

    void AddRemoteCandidate(const std::string & candidate, const std::string & mid) override
    {
        if (mid.empty())
        {
            mPeerConnection->addRemoteCandidate(rtc::Candidate(candidate));
        }
        else
        {
            mPeerConnection->addRemoteCandidate(rtc::Candidate(candidate, mid));
        }
    }

    std::shared_ptr<WebRTCTrack> AddTrack(const std::string & mediaType) override
    {
        if (mediaType == "video")
        {
            rtc::Description::Video media(mediaType, rtc::Description::Direction::SendOnly);
            media.addH264Codec(kVideoH264PayloadType);
            media.setBitrate(kVideoBitRate);
            auto track = mPeerConnection->addTrack(media);
            return std::make_shared<LibDataChannelTrack>(track);
        }
        // TODO: Add audio track support
        return nullptr;
    }

private:
    std::shared_ptr<rtc::PeerConnection> mPeerConnection;
};

class LibDataChannelStack : public WebRTCStack
{
public:
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    std::unique_ptr<WebRTCPeerConnection> CreatePeerConnection() override
    {
        return std::make_unique<LibDataChannelPeerConnection>();
    }
};

} // namespace

std::unique_ptr<WebRTCStack> WebRTCStack::Create()
{
    return std::make_unique<LibDataChannelStack>();
}