#ifndef MYFLIRCAMERA_H
#define MYFLIRCAMERA_H

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

//enum ImageStatus
//{
//     IMAGE_NO_ERROR = 0,
///**< Image is returned from GetNextImage() call without any errors. */
//     IMAGE_CRC_CHECK_FAILED,
///**< Image failed CRC check. */
//     IMAGE_INSUFFICIENT_SIZE,
///**< Image size is smaller than expected. */
//     IMAGE_MISSING_PACKETS,
///**< Image has missing packets */
//     IMAGE_LEADER_BUFFER_SIZE_INCONSISTENT,
///**< Image leader is incomplete. */
//     IMAGE_TRAILER_BUFFER_SIZE_INCONSISTENT,
///**< Image trailer is incomplete. */
//     IMAGE_PACKETID_INCONSISTENT,
///**< Image has an inconsistent packet id. */
//     IMAGE_DATA_INCOMPLETE,
///**< Image data is incomplete. */
//     IMAGE_UNKNOWN_ERROR
///**< Image has an unknown error. */
//};


class myflircamera
{
public:
    myflircamera();
};

#endif // MYFLIRCAMERA_H
