// SPDX-License-Identifier: MIT
pragma solidity ^0.8.13;

import "forge-std/Script.sol";
import "forge-std/console.sol";
import "../src/PngLib.sol";

contract PngTool is Script {
    using PngLib for bytes;

    function run() public {
        string memory mode = vm.envString("MODE");
        string memory filePath = vm.envString("FILE");
        
        // Read file
        bytes memory fileData = vm.readFileBinary(filePath);
        
        // Parse chunks
        PngLib.Chunk[] memory chunks = PngLib.parseChunks(fileData);
        
        if (keccak256(bytes(mode)) == keccak256("print")) {
            printChunks(chunks);
        } else if (keccak256(bytes(mode)) == keccak256("encode")) {
            encode(chunks, filePath);
        } else if (keccak256(bytes(mode)) == keccak256("decode")) {
            decode(chunks);
        } else if (keccak256(bytes(mode)) == keccak256("remove")) {
            remove(chunks, filePath);
        } else {
            console.log("Unknown mode:", mode);
        }
    }

    function printChunks(PngLib.Chunk[] memory chunks) internal view {
        console.log("Chunks found:", chunks.length);
        for(uint i=0; i<chunks.length; i++) {
            console.log("Chunk #", i);
            console.log("  Type:", chunks[i].chunkType);
            console.log("  Length:", chunks[i].length);
            // console.log("  CRC:", chunks[i].crc); // console.log doesn't support bytes4 directly in some versions
        }
    }

    function encode(PngLib.Chunk[] memory chunks, string memory inputPath) internal {
        string memory chunkType = vm.envString("CHUNK_TYPE");
        string memory message = vm.envString("MESSAGE");
        string memory outputPath = vm.envOr("OUTPUT", string.concat(inputPath, ".out.png"));

        // Create new chunk
        PngLib.Chunk memory newChunk = PngLib.createChunk(chunkType, bytes(message));
        
        // Insert before IEND (last chunk)
        // Or just append before IEND.
        // We need to create a new array of size + 1
        PngLib.Chunk[] memory newChunks = new PngLib.Chunk[](chunks.length + 1);
        
        uint insertIndex = chunks.length - 1; // Assuming last is IEND
        // Verify last is IEND?
        if (keccak256(bytes(chunks[chunks.length-1].chunkType)) != keccak256("IEND")) {
             console.log("Warning: Last chunk is not IEND, appending anyway.");
             insertIndex = chunks.length;
        }

        for(uint i=0; i<insertIndex; i++) {
            newChunks[i] = chunks[i];
        }
        newChunks[insertIndex] = newChunk;
        for(uint i=insertIndex; i<chunks.length; i++) {
            newChunks[i+1] = chunks[i];
        }
        
        bytes memory newFileData = PngLib.serialize(newChunks);
        vm.writeFileBinary(outputPath, newFileData);
        console.log("Encoded message into:", outputPath);
    }

    function decode(PngLib.Chunk[] memory chunks) internal view {
        string memory chunkType = vm.envString("CHUNK_TYPE");
        
        bool found = false;
        for(uint i=0; i<chunks.length; i++) {
            if (keccak256(bytes(chunks[i].chunkType)) == keccak256(bytes(chunkType))) {
                console.log("Found message:", string(chunks[i].data));
                found = true;
            }
        }
        if (!found) {
            console.log("No chunk found with type:", chunkType);
        }
    }

    function remove(PngLib.Chunk[] memory chunks, string memory inputPath) internal {
        string memory chunkType = vm.envString("CHUNK_TYPE");
        string memory outputPath = vm.envOr("OUTPUT", string.concat(inputPath, ".clean.png"));
        
        uint count = 0;
        for(uint i=0; i<chunks.length; i++) {
            if (keccak256(bytes(chunks[i].chunkType)) != keccak256(bytes(chunkType))) {
                count++;
            }
        }
        
        if (count == chunks.length) {
            console.log("No chunk found to remove.");
            return;
        }

        PngLib.Chunk[] memory newChunks = new PngLib.Chunk[](count);
        uint j = 0;
        for(uint i=0; i<chunks.length; i++) {
            if (keccak256(bytes(chunks[i].chunkType)) != keccak256(bytes(chunkType))) {
                newChunks[j] = chunks[i];
                j++;
            }
        }
        
        bytes memory newFileData = PngLib.serialize(newChunks);
        vm.writeFileBinary(outputPath, newFileData);
        console.log("Removed message, saved to:", outputPath);
    }
}
