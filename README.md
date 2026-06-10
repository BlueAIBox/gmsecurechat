# GMSecureChat (国密安全通信系统)

GMSecureChat 是一款基于 C++ 和 Qt5 框架开发的跨平台安全通信软件，集成了中国国家商用密码算法（国密 SM2/SM3/SM4 以及 ZUC 祖冲之算法），同时融合了本地 DApp 与基于 Hermes 协议的区块链存证和交互功能。

## 主要特性 (Features)

* **全面采用国密算法**:
  * **SM2**: 非对称加密与数字签名（用于密钥协商、身份认证等）。
  * **SM3**: 密码杂凑算法（用于消息摘要、完整性校验）。
  * **SM4**: 分组密码算法（用于消息的对称加密）。
  * **ZUC (祖冲之算法)**: 序列密码算法（用于流数据加密）。
* **安全即时通讯**: 采用 UDP 协议实现端到端的安全消息和文件传输。
* **安全看板 (Security Dashboard)**: 实时监控安全事件、加解密操作及网络状态。
* **区块链/DApp 集成**: 
  * 内置本地 DApp 服务与前端 Web 页面通信。
  * 包含 `GMAuditLedger.sol` 智能合约，用于通信和关键事件的区块链存证。
  * `hermesbridge` 模块：与 Hermes 协议进行分布式网络或跨链交互。

## 依赖 (Dependencies)

* [Qt 5](https://www.qt.io/) (Core, GUI, Network, Widgets 模块)
* [OpenSSL](https://www.openssl.org/) (默认尝试寻找 `D:/OpenSSL-Win64` 或通过 `OPENSSL_ROOT_DIR` 环境变量配置)
* 支持 C++17 的编译器（例如 MinGW 或 MSVC）

## 编译与运行 (Build & Run)

1. 克隆或下载本仓库代码。
2. 在您的 Qt 开发环境 (Qt Creator) 中打开 `gmsecurechat.pro`。
3. 确保 OpenSSL 库路径正确配置。
4. 执行编译并运行。

或者通过命令行编译：
```bash
qmake gmsecurechat.pro
make
# 运行生成的 gmsecurechat.exe
```

## 目录结构说明 (Structure)

* `/contracts`：智能合约代码目录。
* `/web-dapp`：内置 DApp 的前端页面和脚本。
* `/tools`：附加的辅助开发和数据处理脚本。
* 源文件根目录：包含了 UI 逻辑 (`mainwindow`)、通讯逻辑 (`udpmanager`, `messagehandler`)、国密算法实现 (`sm2`, `sm3`, `sm4`, `zuc`)、以及 DApp 桥接服务 (`localdappserver`, `hermesbridge`) 等核心文件。
