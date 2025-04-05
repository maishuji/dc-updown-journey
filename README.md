# Updown Journey Project
An independent 2D game currently in development for the Sega Dreamcast, following the journey of a human character traveling from top to bottom. 



## Usage

### Running the game on hardware (BBA connection)


We assume the IP address of the Dreamcast is `10.42.0.80`
The dreamcast is connected to the network via a BBA (Broadband Adapter) through the ethernet port of the host machine.

```shell
make run-dc-remote
```


### 🔧 Debugging Dreamcast Network Connection (DHCP Setup)

To allow your Dreamcast to get an IP address automatically over Ethernet (via BBA), you’ll need to set up a lightweight DHCP server using `dnsmasq`.

#### 1. 🧰 Install `dnsmasq`

```bash
sudo apt install dnsmasq
```

---

#### 2. ⚙️ Configure `dnsmasq`

Create a configuration file specifically for the Dreamcast setup:

```bash
sudo nano /etc/dnsmasq.d/dreamcast.conf
```

Add the following content:

```ini
interface=<your-dreamcast-interface>
bind-interfaces

dhcp-range=192.168.0.50,192.168.0.150,12h
dhcp-option=3,192.168.0.1
dhcp-option=6,8.8.8.8,1.1.1.1
```

🔁 **Replace** `<your-dreamcast-interface>` with the name of your Ethernet-to-USB adapter or relevant NIC (e.g., `enxa0cec85e02d8`).  
You can find it using:

```bash
ip addr
```

Look for the interface that is **connected to the Dreamcast** (usually shows as `DOWN` when unplugged, `UP` when plugged in).

---

#### 3. 🔄 Restart `dnsmasq`

```bash
sudo systemctl restart dnsmasq
```

---

#### 4. 📡 Find the Dreamcast IP

While your Dreamcast is booted into `dcload-ip` (or a network-enabled app/game):

```bash
sudo journalctl -u dnsmasq -f
```

Look for a block like:

```text
DHCPDISCOVER(enx...) ...
DHCPOFFER(enx...) 192.168.0.85 ...
DHCPREQUEST ...
DHCPACK ...
```

The IP address offered (e.g., `192.168.0.85`) is the one you’ll use with tools like `dc-tool-ip`.
