/*
 * MediaObject.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "MediaObject.h"

using namespace sampleplayer::input;
using namespace dash::mpd;
using namespace dash::network;
using namespace dash::metrics;

MediaObject::MediaObject    (ISegment *segment, IRepresentation *rep) :
             segment        (segment),
             rep            (rep)
{
    InitializeConditionVariable (&this->stateChanged);
    InitializeCriticalSection   (&this->stateLock);
}
MediaObject::~MediaObject   ()
{
    if(this->state == IN_PROGRESS)
    {
        this->segment->AbortDownload();
        this->OnDownloadStateChanged(ABORTED);
    }
    this->segment->DetachDownloadObserver(this);
    this->WaitFinished();

    DeleteConditionVariable (&this->stateChanged);
    DeleteCriticalSection   (&this->stateLock);
}

void    MediaObject::AbortDownload          ()
{
    this->segment->AbortDownload();
    this->OnDownloadStateChanged(ABORTED);
}
bool    MediaObject::StartDownload          ()
{
    this->segment->AttachDownloadObserver(this);
    return this->segment->StartDownload();
}
void    MediaObject::WaitFinished           ()
{
    EnterCriticalSection(&this->stateLock);

    while(this->state != COMPLETED && this->state != ABORTED)
        SleepConditionVariableCS(&this->stateChanged, &this->stateLock, INFINITE);

    LeaveCriticalSection(&this->stateLock);
}
void    MediaObject::OnDownloadStateChanged (DownloadState state)
{
    EnterCriticalSection(&this->stateLock);

    this->state = state;

    WakeAllConditionVariable(&this->stateChanged);
    LeaveCriticalSection(&this->stateLock);
}
void    MediaObject::OnDownloadRateChanged  (uint64_t bytesDownloaded)
{
}
int     MediaObject::Read                   (uint8_t *data, size_t len)
{
    return this->segment->Read(data, len);
}
int     MediaObject::Peek                   (uint8_t *data, size_t len)
{
    return this->segment->Peek(data, len);
}
int     MediaObject::Peek                   (uint8_t *data, size_t len, size_t offset)
{
    return this->segment->Peek(data, len, offset);
}
IRepresentation*  MediaObject::GetRepresentation()
{
    return this->rep;
}
const std::vector<ITCPConnection *>&    MediaObject::GetTCPConnectionList   () const
{
    return this->segment->GetTCPConnectionList();
}
const std::vector<IHTTPTransaction *>&  MediaObject::GetHTTPTransactionList () const
{
    return this->segment->GetHTTPTransactionList();
}
