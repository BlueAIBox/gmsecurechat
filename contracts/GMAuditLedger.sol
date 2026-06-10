// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

contract GMAuditLedger {
    struct Record {
        uint256 id;
        address submitter;
        string recordType;
        bytes32 sm3Digest;
        bytes32 merkleRoot;
        bytes32 sessionHead;
        string metadata;
        uint256 timestamp;
    }

    uint256 public recordCount;
    mapping(uint256 => Record) private records;
    mapping(address => uint256[]) private recordsBySubmitter;

    event RecordSubmitted(
        uint256 indexed id,
        address indexed submitter,
        bytes32 indexed sm3Digest,
        uint256 timestamp
    );

    function submitRecord(
        string calldata recordType,
        bytes32 sm3Digest,
        bytes32 merkleRoot,
        bytes32 sessionHead,
        string calldata metadata
    ) external returns (uint256 id) {
        id = ++recordCount;
        Record storage record = records[id];
        record.id = id;
        record.submitter = msg.sender;
        record.recordType = recordType;
        record.sm3Digest = sm3Digest;
        record.merkleRoot = merkleRoot;
        record.sessionHead = sessionHead;
        record.metadata = metadata;
        record.timestamp = block.timestamp;

        recordsBySubmitter[msg.sender].push(id);

        emit RecordSubmitted(id, msg.sender, sm3Digest, block.timestamp);
    }

    function getRecord(uint256 id) external view returns (Record memory) {
        require(id > 0 && id <= recordCount, "record not found");
        return records[id];
    }

    function getRecordIdsBySubmitter(address submitter) external view returns (uint256[] memory) {
        return recordsBySubmitter[submitter];
    }
}
