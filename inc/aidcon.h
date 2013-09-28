/*
    This file is a part of the AID (Another Image Debugger) project.

    Copyright (C) 2013  Olinski Krzysztof E.

    This program is free software: you can redistribute it 
    and/or modify it under the terms of the GNU General Public License 
    as published by the Free Software Foundation, either version 3 of the License, 
    or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
    or FITNESS FOR A PARTICULAR PURPOSE. 
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program.  
    If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AIDCON_H
#define AIDCON_H

namespace aidcon{

/* Return values */
const int                RES_OK                   = 0; 
const int                RES_ERROR                = 1;

/* Aux flags */
const unsigned int       FLAG_AUTO_GAIN_BIAS      = 0x01;      // aux flags

/* Public constants */
const unsigned int       ERROR_STRING_MAX_LENGTH  = 1024;        // constraint for an error message length
const unsigned int       FORMAT_STRING_LENGTH     = 128;         // constraint for the pixel format string length
const unsigned int       IMG_NAME_MAX_LENGTH      = 128;         // constraint for an image name length
const unsigned int       IMG_NOTES_MAX_LENGTH     = 4096;        // constraint for an image notes length
const unsigned int       MAX_HOST_NAME            = 256;         // constraint for a host name length

class CTransim
{
public:
           CTransim();
           ~CTransim();

    /*!
     *\brief Sets communication parameters.
     *\param portNumber server port number
     *\param IPStr host name
     *\timeoutInMS connection timeout in ms
     */
    void   setPar(unsigned int portNumber = 5999,
                  const char* IPStr = "127.0.0.1",
                  int timeoutInMS = 30000);

    /*!
     *\brief Sends an image buffer with associated parameters.
     *\param width            image width
     *\param height           image height
     *\param format           pointer to a pixel format string
     *\param sizeInBytes      size of image's source data
     *\param dataButes        pointer to the image's source data buffer
     *\param name             pointer to an image's name string
     *\param notes            pointer to an image's notes string
     *\param rowStrideInBits  row stride for the image's source data
     *\return RES_OK if transmission was successful, RES_ERROR otherwise.
     */
    int    sendImg(unsigned int width,
                   unsigned int height,
                   const char*  format,
                   int sizeInBytes,
                   void* dataBytes,
                   const char* name = 0,
                   const char* notes = 0,
                   unsigned int rowStrideInBits = 0);

    /*!
     *\brief Sets channels gain factors.
     *\param channelNumber   0 for RED, 1 for GREEN, 2 for BLUE, 3 for ALPHA
     *\param fvalue          new gain value
     */
    void   setGain(int channelNumber, float fvalue);

    /*!
     *\brief Sets channels bias factors.
     *\param channelNumber   0 for RED, 1 for GREEN, 2 for BLUE, 3 for ALPHA
     *\param fvalue          new bias value
     */
    void   setBias(int channelNumber, float fvalue);

    /*!
     *\brief Sets auxiliary flags.
     *\param flags   flags value 
     */
    void   setFlags(unsigned int flags);

private:

    unsigned int    portNumber;
    char            hostName[MAX_HOST_NAME];
    unsigned int    timeout;

    unsigned int    auxFlags;

    float           chGain[4];
    float           chBias[4];
};

}//end of namespace dupacon

#endif // AIDCON_H
