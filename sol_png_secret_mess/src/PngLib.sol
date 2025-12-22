// SPDX-License-Identifier: MIT
pragma solidity ^0.8.13;

library PngLib {
    struct Chunk {
        uint32 length;
        string chunkType;
        bytes data;
        bytes4 crc;
    }

    bytes8 constant PNG_SIGNATURE = 0x89504e470d0a1a0a;

    error InvalidPngSignature();
    error InvalidChunkLength();

    function validateHeader(bytes memory fileData) internal pure {
        if (fileData.length < 8) revert InvalidPngSignature();
        bytes8 signature;
        assembly {
            signature := mload(add(fileData, 0x20))
        }
        if (signature != PNG_SIGNATURE) revert InvalidPngSignature();
    }

    function readUint32(bytes memory data, uint256 offset) internal pure returns (uint32) {
        require(data.length >= offset + 4, "Out of bounds");
        uint256 fullWord;
        assembly {
            fullWord := mload(add(add(data, 0x20), offset))
        }
        return uint32(fullWord >> 224);
    }

    function parseChunks(bytes memory fileData) internal pure returns (Chunk[] memory) {
        validateHeader(fileData);
        
        // First pass: count chunks
        uint256 count = 0;
        uint256 offset = 8; // Skip signature
        
        while (offset < fileData.length) {
            uint32 len = readUint32(fileData, offset);
            count++;
            // Length (4) + Type (4) + Data (len) + CRC (4)
            offset += 4 + 4 + len + 4;
        }

        Chunk[] memory chunks = new Chunk[](count);
        
        // Second pass: fill chunks
        offset = 8;
        for (uint256 i = 0; i < count; i++) {
            uint32 len = readUint32(fileData, offset);
            offset += 4;
            
            bytes memory typeBytes = new bytes(4);
            for(uint j=0; j<4; j++) typeBytes[j] = fileData[offset+j];
            string memory chunkType = string(typeBytes);
            offset += 4;
            
            bytes memory data = new bytes(len);
            for(uint j=0; j<len; j++) data[j] = fileData[offset+j];
            offset += len;
            
            bytes4 crc;
            // Read CRC
             bytes32 crcWord;
            assembly {
                crcWord := mload(add(add(fileData, 0x20), offset))
            }
            // CRC is 4 bytes. mload gets 32 bytes. We want the first 4.
            crc = bytes4(crcWord);
            offset += 4;
            
            chunks[i] = Chunk(len, chunkType, data, crc);
        }
        
        return chunks;
    }

    function calculateCRC(bytes memory data) internal pure returns (bytes4) {
        uint32 crc = 0xFFFFFFFF;
        for (uint i = 0; i < data.length; i++) {
            uint32 byteVal = uint32(uint8(data[i]));
            crc = crc ^ byteVal;
            for (uint j = 0; j < 8; j++) {
                if ((crc & 1) != 0) {
                    crc = (crc >> 1) ^ 0xEDB88320;
                } else {
                    crc = crc >> 1;
                }
            }
        }
        return bytes4(crc ^ 0xFFFFFFFF);
    }

    function createChunk(string memory chunkType, bytes memory data) internal pure returns (Chunk memory) {
        bytes memory typeBytes = bytes(chunkType);
        require(typeBytes.length == 4, "Invalid type length");
        
        // CRC is calculated over Type + Data
        bytes memory crcInput = new bytes(4 + data.length);
        for(uint i=0; i<4; i++) crcInput[i] = typeBytes[i];
        for(uint i=0; i<data.length; i++) crcInput[4+i] = data[i];
        
        bytes4 crc = calculateCRC(crcInput);
        
        return Chunk(uint32(data.length), chunkType, data, crc);
    }

    function serialize(Chunk[] memory chunks) internal pure returns (bytes memory) {
        // Calculate total size
        uint256 totalSize = 8; // Signature
        for(uint i=0; i<chunks.length; i++) {
            totalSize += 4 + 4 + chunks[i].data.length + 4;
        }
        
        bytes memory fileData = new bytes(totalSize);
        
        // Write signature
        bytes8 sig = PNG_SIGNATURE;
        assembly {
            mstore(add(fileData, 0x20), sig)
        }
        
        uint256 offset = 8;
        for(uint i=0; i<chunks.length; i++) {
            Chunk memory chunk = chunks[i];
            
            // Length (big endian)
            uint32 len = chunk.length;
            fileData[offset] = bytes1(uint8(len >> 24));
            fileData[offset+1] = bytes1(uint8(len >> 16));
            fileData[offset+2] = bytes1(uint8(len >> 8));
            fileData[offset+3] = bytes1(uint8(len));
            offset += 4;
            
            // Type
            bytes memory typeBytes = bytes(chunk.chunkType);
            for(uint j=0; j<4; j++) fileData[offset+j] = typeBytes[j];
            offset += 4;
            
            // Data
            for(uint j=0; j<chunk.data.length; j++) fileData[offset+j] = chunk.data[j];
            offset += chunk.data.length;
            
            // CRC
            fileData[offset] = chunk.crc[0];
            fileData[offset+1] = chunk.crc[1];
            fileData[offset+2] = chunk.crc[2];
            fileData[offset+3] = chunk.crc[3];
            offset += 4;
        }
        
        return fileData;
    }
}
