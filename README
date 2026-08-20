# Xv6 操作系统实验项目

基于 MIT 6.828 的 xv6-labs-2021 完成的操作系统实验，共 10 个实验，每个实验使用独立 Git 分支管理。

## 实验完成情况

| 实验   | 分支    | 主要内容                             | 评分    |
| ------ | ------- | ------------------------------------ | ------- |
| 实验一 | util    | sleep、pingpong、primes、find、xargs | 100/100 |
| 实验二 | syscall | trace、sysinfo                       | 35/35   |
| 实验三 | pgtbl   | vmprint、sys_pgaccess                | 46/46   |
| 实验四 | traps   | backtrace、sigalarm/sigreturn        | 85/85   |
| 实验五 | cow     | Copy-on-write fork                   | 110/110 |
| 实验六 | thread  | uthread、ph、barrier                 | 60/60   |
| 实验七 | net     | E1000 网卡驱动                       | 100/100 |
| 实验八 | lock    | 内存分配器、缓冲区缓存               | 70/70   |
| 实验九 | fs      | 大文件、符号链接                     | 100/100 |
| 实验十 | mmap    | mmap/munmap、懒加载、MAP_SHARED      | 140/140 |

## 环境

- Windows 11 + WSL2
- Ubuntu 22.04.5 LTS
- riscv64-unknown-elf-gcc 10.2.0
- QEMU 7.2.0

## 编译与运行

```bash
cd xv6-labs-2021
git checkout <分支名>   # 例如 util、syscall、mmap
make qemu               # 编译并启动 xv6
make grade              # 运行当前实验评分
```

实验七网络驱动需要使用带 user 网络后端的 QEMU：

```bash
make grade QEMU=/home/rjc/qemu-slirp/bin/qemu-system-riscv64
```

## 分支说明

每个实验的最新实现位于对应分支的最新提交：

```text
util
syscall
pgtbl
traps
cow
thread
net
lock
fs
mmap
```

## 致谢

- MIT 6.828 Operating System Engineering
- xv6 教学操作系统：https://pdos.csail.mit.edu/6.828/2021/xv6.html
- 原始实验仓库：git://g.csail.mit.edu/xv6-labs-2021
