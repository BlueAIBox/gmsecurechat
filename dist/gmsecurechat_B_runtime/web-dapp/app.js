const SEPOLIA_CHAIN_ID = "0xaa36a7";
const DEFAULT_ACCOUNT = "0x7bD441c4C131901dA79a04B0a8137e029754c9DE";

const abi = [
  {
    "inputs": [
      { "internalType": "string", "name": "recordType", "type": "string" },
      { "internalType": "bytes32", "name": "sm3Digest", "type": "bytes32" },
      { "internalType": "bytes32", "name": "merkleRoot", "type": "bytes32" },
      { "internalType": "bytes32", "name": "sessionHead", "type": "bytes32" },
      { "internalType": "string", "name": "metadata", "type": "string" }
    ],
    "name": "submitRecord",
    "outputs": [{ "internalType": "uint256", "name": "id", "type": "uint256" }],
    "stateMutability": "nonpayable",
    "type": "function"
  }
];

let account = "";
let walletSignature = "0x";
let signedMessage = "";
let expectedAccount = DEFAULT_ACCOUNT;
let transferPayload = null;
let walletBindPayload = null;

const $ = (id) => document.getElementById(id);

function log(value) {
  const text = typeof value === "string" ? value : JSON.stringify(value, null, 2);
  $("output").textContent = `${new Date().toLocaleTimeString()}\n${text}\n\n${$("output").textContent}`;
}

async function notifyQt(type, payload) {
  const params = Object.entries({ type, ...payload })
    .map(([key, value]) => `${encodeURIComponent(key)}=${encodeURIComponent(value ?? "")}`)
    .join("&");
  try {
    await fetch(`/callback?${params}`, { cache: "no-store" });
  } catch {
    // Qt callback is a local convenience channel; transfer itself already happened.
  }
}

function stripHex(value) {
  return (value || "").trim().replace(/^0x/i, "").replace(/[^0-9a-f]/gi, "");
}

function bytes32(value) {
  const hex = stripHex(value);
  return `0x${hex.padEnd(64, "0").slice(0, 64)}`;
}

function utf8Hex(value) {
  return `0x${Array.from(new TextEncoder().encode(value))
    .map((b) => b.toString(16).padStart(2, "0"))
    .join("")}`;
}

function rot64(value, shift) {
  const mask = (1n << 64n) - 1n;
  const s = BigInt(shift);
  if (s === 0n) return value & mask;
  return ((value << s) | (value >> (64n - s))) & mask;
}

function keccak256Hex(text) {
  const rounds = [
    0x0000000000000001n, 0x0000000000008082n, 0x800000000000808an,
    0x8000000080008000n, 0x000000000000808bn, 0x0000000080000001n,
    0x8000000080008081n, 0x8000000000008009n, 0x000000000000008an,
    0x0000000000000088n, 0x0000000080008009n, 0x000000008000000an,
    0x000000008000808bn, 0x800000000000008bn, 0x8000000000008089n,
    0x8000000000008003n, 0x8000000000008002n, 0x8000000000000080n,
    0x000000000000800an, 0x800000008000000an, 0x8000000080008081n,
    0x8000000000008080n, 0x0000000080000001n, 0x8000000080008008n
  ];
  const rotation = [
    0, 1, 62, 28, 27,
    36, 44, 6, 55, 20,
    3, 10, 43, 25, 39,
    41, 45, 15, 21, 8,
    18, 2, 61, 56, 14
  ];
  const mask = (1n << 64n) - 1n;
  const state = new Array(25).fill(0n);
  const bytes = Array.from(new TextEncoder().encode(text));
  const rate = 136;
  bytes.push(0x01);
  while ((bytes.length % rate) !== rate - 1) bytes.push(0);
  bytes.push(0x80);

  function permute() {
    for (const rc of rounds) {
      const c = new Array(5);
      const d = new Array(5);
      for (let x = 0; x < 5; x++) {
        c[x] = state[x] ^ state[x + 5] ^ state[x + 10] ^ state[x + 15] ^ state[x + 20];
      }
      for (let x = 0; x < 5; x++) {
        d[x] = c[(x + 4) % 5] ^ rot64(c[(x + 1) % 5], 1);
      }
      for (let x = 0; x < 5; x++) {
        for (let y = 0; y < 5; y++) {
          state[x + 5 * y] = (state[x + 5 * y] ^ d[x]) & mask;
        }
      }

      const b = new Array(25);
      for (let x = 0; x < 5; x++) {
        for (let y = 0; y < 5; y++) {
          b[y + 5 * ((2 * x + 3 * y) % 5)] = rot64(state[x + 5 * y], rotation[x + 5 * y]);
        }
      }
      for (let x = 0; x < 5; x++) {
        for (let y = 0; y < 5; y++) {
          state[x + 5 * y] = (b[x + 5 * y] ^ ((~b[((x + 1) % 5) + 5 * y] & mask) & b[((x + 2) % 5) + 5 * y])) & mask;
        }
      }
      state[0] = (state[0] ^ rc) & mask;
    }
  }

  for (let block = 0; block < bytes.length; block += rate) {
    for (let i = 0; i < rate; i++) {
      const lane = Math.floor(i / 8);
      const shift = BigInt((i % 8) * 8);
      state[lane] ^= BigInt(bytes[block + i]) << shift;
    }
    permute();
  }

  const out = [];
  for (let i = 0; i < 32; i++) {
    const lane = Math.floor(i / 8);
    const shift = BigInt((i % 8) * 8);
    out.push(Number((state[lane] >> shift) & 0xffn).toString(16).padStart(2, "0"));
  }
  return out.join("");
}

function pad32(hex) {
  const clean = stripHex(hex);
  return clean.padStart(64, "0");
}

function encodeUint256(value) {
  return BigInt(value).toString(16).padStart(64, "0");
}

function encodeAddress(value) {
  return pad32(value);
}

function encodeBytes32(value) {
  return stripHex(value).padStart(64, "0").slice(0, 64);
}

function encodeDynamicBytes(hexValue) {
  const clean = stripHex(hexValue);
  const length = clean.length / 2;
  const paddedLength = Math.ceil(length / 32) * 64;
  return encodeUint256(length) + clean.padEnd(paddedLength, "0");
}

function encodeString(value) {
  return encodeDynamicBytes(utf8Hex(value));
}

function hexQuantity(value) {
  return `0x${value.toString(16)}`;
}

function parseEthToWei(value) {
  const text = (value || "").trim();
  if (!/^(0|[1-9]\d*)(\.\d{1,18})?$/.test(text)) {
    throw new Error("金额格式不正确，最多支持 18 位小数。");
  }
  const [whole, fraction = ""] = text.split(".");
  const wei = BigInt(whole) * 1000000000000000000n + BigInt(fraction.padEnd(18, "0"));
  if (wei <= 0n) {
    throw new Error("转账金额必须大于 0。");
  }
  return wei;
}

function isAddress(value) {
  return /^0x[0-9a-fA-F]{40}$/.test((value || "").trim());
}

async function ensureSepolia() {
  const chainId = await ethereum.request({ method: "eth_chainId" });
  if (chainId === SEPOLIA_CHAIN_ID) return;
  await ethereum.request({
    method: "wallet_switchEthereumChain",
    params: [{ chainId: SEPOLIA_CHAIN_ID }]
  });
}

async function connect() {
  if (!window.ethereum) {
    log("未检测到 MetaMask，请用已安装 MetaMask 的 Chrome/Edge 打开本页面。");
    return;
  }
  await ensureSepolia();
  const accounts = await ethereum.request({ method: "eth_requestAccounts" });
  account = accounts[0] || "";
  $("accountText").textContent = account || "未连接";
  $("networkText").textContent = "Sepolia";
  if (account && expectedAccount && account.toLowerCase() !== expectedAccount.toLowerCase()) {
    log(`当前钱包是 ${account}，期望地址是 ${expectedAccount}。如果不是故意切换，请在 MetaMask 里选回该账户。`);
  } else {
    log(`已连接 ${account}`);
  }
}

function getContractAddress() {
  return ($("contractAddress").value || localStorage.getItem("gmAuditContract") || "").trim();
}

function saveContract() {
  const address = $("contractAddress").value.trim();
  if (!/^0x[0-9a-fA-F]{40}$/.test(address)) {
    log("合约地址格式不对，请粘贴 0x 开头的 40 字节地址。");
    return;
  }
  localStorage.setItem("gmAuditContract", address);
  $("contractText").textContent = address;
  log(`已保存合约地址：${address}`);
}

function collectRecord() {
  const metadata = $("metadata").value.trim() || "{}";
  try {
    JSON.parse(metadata);
  } catch {
    throw new Error("元数据 JSON 格式不正确。");
  }
  return {
    recordType: $("recordType").value,
    sm3Digest: bytes32($("sm3Digest").value),
    merkleRoot: bytes32($("merkleRoot").value),
    sessionHead: bytes32($("sessionHead").value),
    gmFingerprint: $("gmFingerprint").value.trim(),
    peerFingerprint: $("peerFingerprint").value.trim(),
    metadata
  };
}

function buildChainMetadata(record) {
  return JSON.stringify({
    source: "gmsecurechat",
    gmFingerprint: record.gmFingerprint,
    peerFingerprint: record.peerFingerprint,
    signedBy: account,
    walletSignature,
    signedMessage,
    metadata: JSON.parse(record.metadata)
  });
}

function fillFromLedgerRecord(record) {
  const payload = record.payload || {};
  $("recordType").value = record.type || $("recordType").value;
  $("sm3Digest").value = payload.fileHashSM3 || payload.bindingSM3 || payload.messageSM3 || payload.originalFileSM3 || record.recordHash || "";
  $("merkleRoot").value = payload.merkleRootSM3 || payload.originalMerkleRoot || "";
  $("sessionHead").value = payload.chainHead || payload.ledgerHead || "";
  $("gmFingerprint").value = payload.ownerFingerprint || payload.communicationFingerprint || payload.gmFingerprint || "";
  $("peerFingerprint").value = payload.peerFingerprint || "";
  $("metadata").value = JSON.stringify({ source: "gmsecurechat", importedRecord: record }, null, 2);
  log({ imported: record.type, recordHash: record.recordHash, payload });
}

function loadRecordFromQuery() {
  const params = new URLSearchParams(location.search);
  const recordText = params.get("record");
  if (!recordText) return;
  const wallet = params.get("wallet");
  if (wallet) {
    expectedAccount = wallet;
    log(`期望钱包地址：${wallet}`);
  }
  const record = JSON.parse(recordText);
  fillFromLedgerRecord(record);
  $("autoPanel").classList.remove("hidden");
}

async function importLedgerFile(event) {
  const file = event.target.files && event.target.files[0];
  if (!file) return;
  const text = await file.text();
  const lines = text.split(/\r?\n/).map((line) => line.trim()).filter(Boolean);
  if (!lines.length) {
    log("存证文件为空。");
    return;
  }
  const lastRecord = JSON.parse(lines[lines.length - 1]);
  fillFromLedgerRecord(lastRecord);
}

function fillTransferPanel(transfer) {
  transferPayload = transfer;
  expectedAccount = transfer.fromWallet || expectedAccount;
  $("transferFrom").value = transfer.fromWallet || "";
  $("transferTo").value = transfer.toWallet || "";
  $("transferAmount").value = transfer.amountEth || "0.001";
  $("transferPeer").value = `${transfer.toNick || "peer"} / ${transfer.recipientIp || ""}`;
  $("transferDigest").value = transfer.transferSM3 || "";
  $("transferEvidence").value = JSON.stringify(transfer, null, 2);
  $("transferPanel").classList.remove("hidden");
  log({ transferLoaded: true, to: transfer.toWallet, amountEth: transfer.amountEth, sm3: transfer.transferSM3 });
}

function fillWalletBindPanel(payload) {
  walletBindPayload = payload;
  expectedAccount = payload.wallet || expectedAccount;
  $("walletBindRequester").value = payload.requester || "";
  $("walletBindAddress").value = payload.wallet || "";
  $("walletBindNonce").value = payload.nonce || "";
  $("walletBindMessage").value = payload.message || "";
  $("walletBindPanel").classList.remove("hidden");
  log({
    walletBindingLoaded: true,
    requester: payload.requester,
    wallet: payload.wallet,
    nonce: payload.nonce
  });
}

function loadWalletBindFromQuery() {
  const params = new URLSearchParams(location.search);
  if (params.get("mode") !== "wallet_bind") return;
  const wallet = params.get("wallet") || "";
  if (wallet) expectedAccount = wallet;
  fillWalletBindPanel({
    nonce: params.get("nonce") || "",
    wallet,
    requester: params.get("requester") || "",
    subject: params.get("subject") || "",
    message: params.get("message") || ""
  });
}

function loadTransferFromQuery() {
  const params = new URLSearchParams(location.search);
  if (params.get("mode") !== "transfer") return;
  const wallet = params.get("wallet");
  if (wallet) expectedAccount = wallet;
  const transferText = params.get("transfer");
  if (!transferText) return;
  fillTransferPanel(JSON.parse(transferText));
}

async function signWalletBinding() {
  if (!walletBindPayload) {
    log("没有可用的钱包归属验证请求。");
    return;
  }
  if (!account) await connect();
  await ensureSepolia();

  const wallet = (walletBindPayload.wallet || "").trim();
  if (!isAddress(wallet)) {
    log("待验证钱包地址格式不正确。");
    return;
  }
  if (account.toLowerCase() !== wallet.toLowerCase()) {
    log(`MetaMask 当前账户 ${account} 与待验证钱包 ${wallet} 不一致，请先切换账户。`);
    return;
  }

  const message = walletBindPayload.message || "";
  const walletBindSignature = await ethereum.request({
    method: "personal_sign",
    params: [message, account]
  });

  let walletRecoveredAddress = "";
  try {
    walletRecoveredAddress = await ethereum.request({
      method: "personal_ecRecover",
      params: [message, walletBindSignature]
    });
  } catch (err) {
    log(`MetaMask 地址恢复接口不可用：${err.message || err}`);
  }

  log({
    walletBindingSigned: true,
    account,
    nonce: walletBindPayload.nonce,
    walletBindSignature,
    walletRecoveredAddress,
    message
  });

  await notifyQt("wallet_bind_signature", {
    nonce: walletBindPayload.nonce || "",
    wallet,
    account,
    walletSignature: walletBindSignature,
    walletRecoveredAddress,
    message
  });
  log("钱包归属签名已回传 GM Secure Chat，请回到 exe 查看验证结果。");
}

async function signRecord() {
  if (!account) await connect();
  const record = collectRecord();
  const message = [
    "GM Secure Chat Sepolia Audit",
    `recordType=${record.recordType}`,
    `sm3Digest=${record.sm3Digest}`,
    `merkleRoot=${record.merkleRoot}`,
    `sessionHead=${record.sessionHead}`,
    `gmFingerprint=${record.gmFingerprint}`,
    `peerFingerprint=${record.peerFingerprint}`,
    `metadata=${record.metadata}`
  ].join("\n");

  walletSignature = await ethereum.request({
    method: "personal_sign",
    params: [message, account]
  });
  signedMessage = message;
  log({ signedBy: account, walletSignature, message });
}

function buildSubmitRecordCalldata(record, chainMetadata) {
  const selector = `0x${keccak256Hex("submitRecord(string,bytes32,bytes32,bytes32,string)").slice(0, 8)}`;
  const dynamicParts = [
    encodeString(record.recordType),
    encodeString(chainMetadata)
  ];
  let offset = 5 * 32;
  const recordTypeOffset = offset; offset += dynamicParts[0].length / 2;
  const metadataOffset = offset;

  return selector +
    encodeUint256(recordTypeOffset) +
    encodeBytes32(record.sm3Digest) +
    encodeBytes32(record.merkleRoot) +
    encodeBytes32(record.sessionHead) +
    encodeUint256(metadataOffset) +
    dynamicParts.join("");
}

async function sendAuditRecord(record, chainMetadata) {
  const contract = getContractAddress();
  if (!/^0x[0-9a-fA-F]{40}$/.test(contract)) {
    throw new Error("请先填写并保存 Sepolia 合约地址。");
  }
  const calldata = buildSubmitRecordCalldata(record, chainMetadata);
  return await ethereum.request({
    method: "eth_sendTransaction",
    params: [{ from: account, to: contract, data: calldata }]
  });
}

async function submitRecord() {
  if (!account) await connect();
  await ensureSepolia();
  const record = collectRecord();
  if (walletSignature === "0x") {
    await signRecord();
  }

  try {
    const txHash = await sendAuditRecord(record, buildChainMetadata(record));
    log({ txHash, explorer: `https://sepolia.etherscan.io/tx/${txHash}` });
  } catch (err) {
    log(err.message || err);
  }
}

async function sendTransfer() {
  if (!transferPayload) {
    log("没有可用的转账请求。");
    return;
  }
  if (!account) await connect();
  await ensureSepolia();

  const from = transferPayload.fromWallet;
  const to = $("transferTo").value.trim();
  const amountEth = $("transferAmount").value.trim();
  if (!isAddress(from) || !isAddress(to)) {
    log("转账地址格式不正确。");
    return;
  }
  if (account.toLowerCase() !== from.toLowerCase()) {
    log(`MetaMask 当前账户 ${account} 与转账发起方 ${from} 不一致，请先切换账户。`);
    return;
  }

  let value;
  try {
    value = hexQuantity(parseEthToWei(amountEth));
  } catch (err) {
    log(err.message || err);
    return;
  }
  let txHash;
  try {
    txHash = await ethereum.request({
      method: "eth_sendTransaction",
      params: [{ from: account, to, value }]
    });
  } catch (err) {
    log(`转账未完成：${err.message || err}`);
    return;
  }
  const transferExplorer = `https://sepolia.etherscan.io/tx/${txHash}`;
  log({ transferTxHash: txHash, explorer: transferExplorer });
  await notifyQt("transfer_success", {
    transferTxHash: txHash,
    amountEth,
    fromWallet: from,
    toWallet: to,
    toNick: transferPayload.toNick || "",
    recipientIp: transferPayload.recipientIp || "",
    transferSM3: transferPayload.transferSM3 || ""
  });

  const auditRecord = {
    recordType: "lan_wallet_transfer",
    sm3Digest: bytes32(transferPayload.transferSM3 || ""),
    merkleRoot: bytes32(""),
    sessionHead: bytes32(transferPayload.sessionHead || ""),
    gmFingerprint: transferPayload.senderFingerprint || "",
    peerFingerprint: transferPayload.recipientFingerprint || "",
    metadata: "{}"
  };
  const chainMetadata = JSON.stringify({
    source: "gmsecurechat",
    action: "lan_wallet_transfer",
    transferTxHash: txHash,
    transferExplorer,
    amountEth,
    fromWallet: from,
    toWallet: to,
    transfer: transferPayload
  });

  try {
    const auditTxHash = await sendAuditRecord(auditRecord, chainMetadata);
    log({
      auditTxHash,
      auditExplorer: `https://sepolia.etherscan.io/tx/${auditTxHash}`,
      note: "转账交易和国密转账证据均已提交 Sepolia"
    });
    await notifyQt("transfer_audit", {
      auditTxHash,
      transferTxHash: txHash,
      amountEth,
      fromWallet: from,
      toWallet: to,
      toNick: transferPayload.toNick || "",
      recipientIp: transferPayload.recipientIp || ""
    });
  } catch (err) {
    log(`转账已发出，但存证未提交：${err.message || err}`);
  }
}

function init() {
  $("contractAddress").value = localStorage.getItem("gmAuditContract") || "";
  $("contractText").textContent = $("contractAddress").value || "未设置";
  $("metadata").value = JSON.stringify({
    source: "gmsecurechat",
    wallet: DEFAULT_ACCOUNT,
    note: "Sepolia audit demo"
  }, null, 2);

  $("connectBtn").addEventListener("click", connect);
  $("saveContractBtn").addEventListener("click", saveContract);
  $("ledgerFile").addEventListener("change", importLedgerFile);
  $("signBtn").addEventListener("click", signRecord);
  $("submitBtn").addEventListener("click", submitRecord);
  $("confirmChainBtn").addEventListener("click", submitRecord);
  $("sendTransferBtn").addEventListener("click", sendTransfer);
  $("signWalletBindBtn").addEventListener("click", signWalletBinding);
  loadRecordFromQuery();
  loadWalletBindFromQuery();
  loadTransferFromQuery();
}

init();
