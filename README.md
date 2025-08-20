# 🔑 KVS Kernel Module

This project implements a **Key-Value Store (KVS)** inside a **Linux Kernel Module** with TCP communication.  
It accepts client requests (`put` / `get`), executes a CPU-intensive task, and responds with `ok`.  
The storage is backed by loop devices (`/dev/loopXX.img`) that act as the database.

---

## ⚙️ Installation

### 1. Clone the repository
### 2. Build the module
  make 
### 3. Insert the module
  sudo insmod kmodule.ko n=5 PORT=12345
  
  n → number of requests accepted from each client (default test value: 5)
  PORT → TCP port for client communication (default test value: 12345)
### 4. Database Initialization
The KVS uses loop devices as storage backends.
Run the provided script to create multiple loop images under /dev:./run.sh
./run.sh
By default, ~900 loop devices are created (/dev/loop39.img … /dev/loop999.img).
You can adjust the number in the script.
### 4. Usage
Simple test with netcat
Connect to the KVS server:
  nc 127.0.0.1 12345
Send requests:
  put key value
  get key
### 5.Switching the Database
You can switch to a different loop device (database) at runtime:
echo 39 | sudo tee /sys/module/kmodule/parameters/loop
This will make the active DB /dev/loop39.img.
### 6.Cleanup
To remove the module:
sudo rmmod kmodule
