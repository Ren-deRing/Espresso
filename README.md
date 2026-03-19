# Espresso
**Espresso**는 x86-64 아키텍처로 개발된 가볍고 강력한 Wasm 전용 베어메탈 운영체제입니다.


## 🚀 Quick Start

### Prerequisites
#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install -y build-essential clang lld llvm nasm xorriso git qemu-system-x86_64 ovmf
```

### Clone
```bash
git clone --recursive https://github.com/Ren-deRing/Espresso
cd Espresso
```

### Build & Run
```bash
make qemu
```

## Third-party software
- This project uses Limine as a bootloader, which is licensed under the BSD-0-Clause license.
- This project uses wasm3 as a runtime, which is licensed under the MIT license.
