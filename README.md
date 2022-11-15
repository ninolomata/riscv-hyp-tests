# RISC-V Hypervisor Extension Tests
A bare-metal application to test specific features of the RISC-V Hypervisor extension

## Simulation on Chromite core
The following guide explains how to simulate the test application on a RISC-V core generated with [Chromite](https://chromite.readthedocs.io/en/latest/overview.html) generator, using [Verilator](https://verilator.org/guide/latest/install.html).

It is assumed that you have already built the Chromite core and created the corresponding verilated executable, which is simulates the core's behavior. If not, you must do it following this [guide](https://chromite.readthedocs.io/en/latest/getting_started.html#).

After performing the required steps, you should have the following files in the **`bin`** folder of your Chromite directory:
- ***chromite_core***: The verilated executable file
- ***boot.mem***: BootROM initialization file

### 1) Clone the repository and checkout to branch *incore*
```
git clone https://github.com/ninolomata/riscv-hyp-tests.git && cd riscv-hyp-tests
git checkout incore
```

### 2) Compile the tests application
> **_:notebook: Note:_** In order to compile the tests you need the *riscv64-unknown-elf-* toolchain, which can be downloaded [here](https://static.dev.sifive.com/dev-tools/freedom-tools/v2020.08/riscv64-unknown-elf-gcc-10.1.0-2020.08.2-x86_64-linux-ubuntu14.tar.gz). The toolchain path must be on the PATH environment variable.

On the top directory of the repo:
```
make PLAT=incore LOG_LEVEL=LOG_DEBUG
```
An output file is generated in ***./build/incore/rvh_test.elf***

### 3) Simulate the tests application with the verilated model
Go to the `bin` folder of your Chromite directory, where the ***chromite_core*** file resides:
```
cd <path-to-chromite>/bin
```
Then run the verilated model of the core, indicating the .elf output file of the compiled aplication:
```
./chromite_core +elf=<*path-to-riscv-hyp-tests*>/build/incore/rvh_test.elf
```
As referred in the Chromite documentation [notes](https://chromite.readthedocs.io/en/latest/getting_started.html#notes-on-simulation), the simulation model of the core contains a simple UART, which uses the putchar function defined in the incore platform folder. 

The output of the simulation is captured in the ***app_log*** file, located in the `bin` folder of your Chromite directory.

### Enabling/Disabling tests
By commenting/modifying the tests defined in `test_register.c` it is possible to select which tests will be performed by the application.
