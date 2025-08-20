# 🔑 KVS Kernel Module

A **Key-Value Store (KVS)** implementation inside a **Linux Kernel Module** with TCP communication support. This module accepts client requests (`put`/`get`), executes CPU-intensive tasks, and responds with acknowledgments. The storage backend uses loop devices (`/dev/loopXX.img`) acting as persistent databases.

## ⚙️ Installation

### 1. Clone the Repository
```bash
git clone <repository-url>
cd kvs-kernel-module
```

### 2. Build the Module
```bash
make
```

### 3. Insert the Module
```bash
sudo insmod kmodule.ko n=5 PORT=12345
```

**Parameters:**
- `n` → Number of requests accepted from each client (default: 5)
- `PORT` → TCP port for client communication (default: 12345)

### 4. Database Initialization
The KVS uses loop devices as storage backends. Run the provided script to create multiple loop images:

```bash
./run.sh
```

By default, approximately 900 loop devices are created (`/dev/loop39.img` to `/dev/loop999.img`). You can adjust the number of devices in the script as needed.

## 📖 Usage

### Basic Testing with Netcat

1. **Connect to the KVS server:**
```bash
nc 127.0.0.1 12345
```

2. **Send requests:**
```bash
put key value    # Store a key-value pair
get key          # Retrieve value for a key
```

3. **Example session:**
```bash
$ nc 127.0.0.1 12345
put name alice
ok
get name
alice
put age 25
ok
get age
25
```

### Runtime Database Switching

You can switch to a different loop device (database) at runtime without stopping the service:

```bash
echo 39 | sudo tee /sys/module/kmodule/parameters/loop
```

This command makes `/dev/loop39.img` the active database. All subsequent operations will use this storage backend.

### Monitoring Module Status

Check module parameters and status:
```bash
cat /sys/module/kmodule/parameters/loop    # Current active loop device
```

## 🔧 Configuration

### Module Parameters

| Parameter | Description | Default | Example |
|-----------|-------------|---------|---------|
| `n` | Max requests per client | 5 | `n=10` |
| `PORT` | TCP listening port | 12345 | `PORT=8080` |
| `loop` | Active loop device number | 39 | `loop=50` |


## 🧹 Cleanup

### Remove the Module
```bash
sudo rmmod kmodule
```

### Clean Loop Devices (Optional)
If you want to remove all created loop devices:
```bash
# Add cleanup commands to remove loop devices if needed
sudo losetup -d /dev/loop{39..999} 2>/dev/null || true
```

### Debug Information

Check kernel logs for module messages:
```bash
dmesg | tail -20
```


## ⚠️ Important Notes

- This module runs in kernel space - be cautious with modifications
- Always test in a safe environment before production use
- The CPU-intensive tasks may affect system performance
- Ensure proper cleanup before system shutdown
- Loop devices consume disk space - monitor usage

