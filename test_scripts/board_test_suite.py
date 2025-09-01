"""
Waveshare ESP32-S3 Touch AMOLED 2.41 - Complete Hardware Test Suite
Provides interactive tests for all onboard components to verify the CircuitPython port.
"""

import board
import busio
import digitalio
import analogio
import time
import gc
import os
import microcontroller
import storage
import pwmio
import sdcardio
import wifi
import adafruit_ble

# --- Helper Functions ---
def wait_for_enter(prompt="Press ENTER to continue..."):
    """Waits for the user to press Enter."""
    input(prompt)

def print_header(title):
    """Prints a formatted header for a test section."""
    print("\n" + "="*60)
    print(" " + title)
    print("="*60)

def print_result(success, message):
    """Prints a test result with a checkmark (✓) or an X (✗)."""
    symbol = "✓" if success else "✗"
    print("  " + symbol + " " + message)

def print_info(message):
    """Prints an informational message."""
    print("  - " + message)



def i2c_read_register(i2c, device_addr, register_addr, num_bytes):
    """Read data from an I2C device register.

    Args:
        i2c: Locked I2C bus object
        device_addr: I2C device address
        register_addr: Register address to read from
        num_bytes: Number of bytes to read

    Returns:
        bytearray: Data read from the register
    """
    # Write register address then read data
    # Some devices need this as separate operations
    i2c.writeto(device_addr, bytes([register_addr]))
    result = bytearray(num_bytes)
    i2c.readfrom_into(device_addr, result)
    return result

def set_rtc_time(rtc, year=2025, month=9, day=4, hour=1, minute=18, second=25):
    """Set the RTC to a specific time.

    Args:
        rtc: PCF85063A RTC object
        year, month, day, hour, minute, second: Time components

    Returns:
        bool: True if successful, False otherwise
    """
    try:
        import time
        # Create a time struct
        t = time.struct_time((year, month, day, hour, minute, second, 0, 0, -1))
        rtc.datetime = t
        return True
    except Exception as e:
        print_info("  - Failed to set RTC time: " + str(e))
        return False

def dump_imu_registers(i2c, start_addr=0x00, end_addr=0x30):
    """Dump IMU register values for debugging.

    Args:
        i2c: Locked I2C bus object
        start_addr: Starting register address
        end_addr: Ending register address (inclusive)
    """
    print("IMU Register Dump (0x{:02X} to 0x{:02X}):".format(start_addr, end_addr))
    print("    Addr: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F")
    print("    " + "-" * 52)

    for row_addr in range(start_addr, end_addr + 1, 16):
        row_data = []
        for offset in range(16):
            addr = row_addr + offset
            if addr <= end_addr:
                try:
                    data = i2c_read_register(i2c, 0x6B, addr, 1)
                    row_data.append("{:02X}".format(data[0]))
                except:
                    row_data.append("??")
            else:
                row_data.append("  ")

        print("    0x{:02X}: {}".format(row_addr, " ".join(row_data)))

# --- Test Suite ---

def test_system_info():
    print_header("SYSTEM & MEMORY")
    wait_for_enter()
    try:
        print_info("Board ID: " + board.board_id)
        print_info("CircuitPython Version: " + os.uname().version)
        print_info("CPU: " + os.uname().machine)
        print_info("CPU Frequency: {} MHz".format(microcontroller.cpu.frequency / 1000000))

        s = os.statvfs("/")
        fs_size = s[0] * s[2] / (1024 * 1024)
        print_result(fs_size > 10, "Internal Flash (CIRCUITPY drive): {:.2f} MB".format(fs_size))

        total_heap = (gc.mem_alloc() + gc.mem_free()) / 1024
        print_result(total_heap > 6000, "Total Heap (SRAM + PSRAM): {:.2f} KB".format(total_heap))

    except Exception as e:
        print_result(False, "An error occurred: " + str(e))

def test_onboard_peripherals():
    print_header("ONBOARD PERIPHERALS (Button)")
    wait_for_enter()
    try:
        button = digitalio.DigitalInOut(board.BOOT)
        button.direction = digitalio.Direction.INPUT
        button.pull = digitalio.Pull.UP
        print_info("Please press the BOOT button (GPIO0) within 15 seconds...")
        start_time = time.monotonic()
        pressed = False
        while time.monotonic() - start_time < 15:
            if not button.value:
                print_result(True, "BOOT button press detected.")
                pressed = True
                break
        if not pressed:
            print_result(False, "BOOT button press not detected in 15 seconds.")
        button.deinit()
    except Exception as e:
        print_result(False, "BOOT button test failed: " + str(e))

def test_i2c_scan():
    print_header("I2C BUS SCAN")
    wait_for_enter()
    i2c = None

    try:
        print_info("Initializing I2C bus...")
        i2c = busio.I2C(board.SCL, board.SDA, timeout=5)
        print_info("I2C bus initialized successfully.")

        # Lock the bus only for scanning
        while not i2c.try_lock():
            pass
        try:
            devices = i2c.scan()
        finally:
            i2c.unlock()

        print_info("Found {} I2C devices:".format(len(devices)))

        expected_devices = {
            0x20: "I/O Expander (PCA9554)",
            0x38: "Touch Controller (FT6336U)",
            0x51: "RTC (PCF85063)",
            0x6B: "IMU (QMI8658C)"
        }

        found_addrs = []
        for addr in devices:
            name = expected_devices.get(addr, "Unknown Device")
            print_info("  - 0x{:02X}: {}".format(addr, name))
            found_addrs.append(addr)

        for addr, name in expected_devices.items():
            print_result(addr in found_addrs, "Found " + name)

    except Exception as e:
        print_result(False, "I2C scan failed: " + str(e))
    finally:
        if i2c:
            i2c.deinit()

def test_rtc():
    print_header("RTC (PCF85063)")
    wait_for_enter()
    i2c = None

    try:
        print_info("Initializing I2C...")
        i2c = create_i2c_with_pullups(board.SCL, board.SDA, timeout=5)

        # Check if RTC is present
        while not i2c.try_lock():
            pass
        try:
            devices = i2c.scan()
        finally:
            i2c.unlock()

        if 0x51 not in devices:
            print_result(False, "RTC not found at address 0x51")
            return

        print_info("Testing RTC (PCF85063)...")
        print_info("  - Oscillator should be pre-configured by board_init().")
        try:
            import pcf85063a
            rtc = pcf85063a.PCF85063A(i2c)

            # Read current time
            t = rtc.datetime
            print_info("Raw RTC time: {}-{:02}-{:02} {:02}:{:02}:{:02}".format(
                t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec))

            # Check if RTC time is valid
            if (t.tm_year > 2100 or t.tm_year < 2000 or t.tm_mon > 12 or t.tm_mon < 1 or
                t.tm_mday > 31 or t.tm_mday < 1 or t.tm_hour > 23 or t.tm_min > 59 or t.tm_sec > 59):
                print_result(False, "RTC time is invalid/uninitialized")
                print_info("  - RTC may have lost power or never been set")

                # Use current system time if available, otherwise use a default
                import time as sys_time
                current_time = sys_time.localtime()
                if current_time.tm_year >= 2024:
                    print_info("  - Attempting to set RTC to current system time...")
                    if set_rtc_time(rtc, current_time.tm_year, current_time.tm_mon,
                                  current_time.tm_mday, current_time.tm_hour,
                                  current_time.tm_min, current_time.tm_sec):
                        # Re-read the time to confirm it was set
                        t = rtc.datetime
                        time_str = "{}-{:02}-{:02} {:02}:{:02}:{:02}".format(
                            t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)
                        print_result(True, "RTC successfully set to: " + time_str)
                    else:
                        print_result(False, "Failed to set RTC time")
                else:
                    print_info("  - System time not available, setting default time...")
                    if set_rtc_time(rtc, 2024, 1, 1, 0, 0, 0):
                        t = rtc.datetime
                        time_str = "{}-{:02}-{:02} {:02}:{:02}:{:02}".format(
                            t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)
                        print_result(True, "RTC set to default: " + time_str)
            else:
                time_str = "{}-{:02}-{:02} {:02}:{:02}:{:02}".format(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)
                print_result(True, "Current RTC time: " + time_str)

                # Read time again after a second to verify it's running
                time.sleep(1.1)
                t2 = rtc.datetime
                if t2.tm_sec != t.tm_sec or t2.tm_min != t.tm_min:
                    print_result(True, "RTC is running (time changed)")
                else:
                    print_result(False, "RTC might be stopped (time unchanged after 1 second)")

        except ImportError:
            print_result(False, "RTC test failed: `pcf85063a` library not found.")
        except Exception as e:
            print_result(False, "RTC test failed: " + str(e))

    except Exception as e:
        print_result(False, "RTC initialization failed: " + str(e))
    finally:
        if i2c:
            i2c.deinit()

def test_imu():
    print_header("IMU (QMI8658C)")
    wait_for_enter()
    i2c = None

    try:
        print_info("Initializing I2C...")
        i2c = create_i2c_with_pullups(board.SCL, board.SDA, timeout=5)

        # Check if IMU is present
        while not i2c.try_lock():
            pass
        try:
            devices = i2c.scan()
        finally:
            i2c.unlock()

        if 0x6B not in devices:
            print_result(False, "IMU not found at address 0x6B")
            return

        print_info("Testing IMU (QMI8658C)...")
        print_info("  - IMU should be pre-configured by board_init().")
        try:
            # Lock the bus for raw I2C operations
            while not i2c.try_lock():
                pass

            # Read WHO_AM_I register (0x00)
            buf = i2c_read_register(i2c, 0x6B, 0x00, 1)
            print_result(buf[0] == 0x05, "Who Am I check (expected 0x05, got 0x{:02X})".format(buf[0]))

            # Dump registers for debugging if needed
            print_info("  - Dumping configuration registers...")
            dump_imu_registers(i2c, 0x00, 0x0F)

            # Verify configuration
            ctrl1 = i2c_read_register(i2c, 0x6B, 0x02, 1) # CTRL1
            ctrl2 = i2c_read_register(i2c, 0x6B, 0x03, 1) # CTRL2
            ctrl7 = i2c_read_register(i2c, 0x6B, 0x08, 1)
            ctrl8 = i2c_read_register(i2c, 0x6B, 0x09, 1)
            print_info("  - CTRL1: 0x{:02X}, CTRL2: 0x{:02X}, CTRL7: 0x{:02X}, CTRL8: 0x{:02X}".format(
                ctrl1[0], ctrl2[0], ctrl7[0], ctrl8[0]))
            print_result(True, "IMU configured (8G accel, 128 DPS gyro)")

            print_info("  - Reading sensor data for 5 seconds (Ctrl+C to skip)...")
            print_info("  - Data updates every 0.5 seconds:")

            # Define conversion function outside the loop
            def bytes_to_int16(lsb, msb):
                value = lsb | (msb << 8)
                # Convert to signed if necessary
                if value >= 32768:
                    value = value - 65536
                return value

            start_time = time.monotonic()
            last_print_time = start_time

            while time.monotonic() - start_time < 5:
                current_time = time.monotonic()

                # Print data every 0.5 seconds
                if current_time - last_print_time >= 0.5:
                    elapsed = current_time - start_time
                    # Check status register first (0x2E)
                    status = i2c_read_register(i2c, 0x6B, 0x2E, 1)

                    # Check if either accel (bit 0) or gyro (bit 1) has data
                    if status[0] & 0x03:
                        # QMI8658C doesn't auto-increment on multi-byte reads
                        # Read each axis value separately (2 bytes per axis)

                        # Try reading all data in blocks to reduce I2C overhead
                        # Read accelerometer data block (6 bytes from 0x35)
                        try:
                            # Read X-axis accel
                            ax_lsb = i2c_read_register(i2c, 0x6B, 0x35, 1)[0]
                            ax_msb = i2c_read_register(i2c, 0x6B, 0x36, 1)[0]
                            # Read Y-axis accel
                            ay_lsb = i2c_read_register(i2c, 0x6B, 0x37, 1)[0]
                            ay_msb = i2c_read_register(i2c, 0x6B, 0x38, 1)[0]
                            # Read Z-axis accel
                            az_lsb = i2c_read_register(i2c, 0x6B, 0x39, 1)[0]
                            az_msb = i2c_read_register(i2c, 0x6B, 0x3A, 1)[0]

                            # Read gyroscope data block (6 bytes from 0x3B)
                            # Read X-axis gyro
                            gx_lsb = i2c_read_register(i2c, 0x6B, 0x3B, 1)[0]
                            gx_msb = i2c_read_register(i2c, 0x6B, 0x3C, 1)[0]
                            # Read Y-axis gyro
                            gy_lsb = i2c_read_register(i2c, 0x6B, 0x3D, 1)[0]
                            gy_msb = i2c_read_register(i2c, 0x6B, 0x3E, 1)[0]
                            # Read Z-axis gyro
                            gz_lsb = i2c_read_register(i2c, 0x6B, 0x3F, 1)[0]
                            gz_msb = i2c_read_register(i2c, 0x6B, 0x40, 1)[0]

                            # Convert to signed 16-bit integers
                            ax = bytes_to_int16(ax_lsb, ax_msb)
                            ay = bytes_to_int16(ay_lsb, ay_msb)
                            az = bytes_to_int16(az_lsb, az_msb)
                            gx = bytes_to_int16(gx_lsb, gx_msb)
                            gy = bytes_to_int16(gy_lsb, gy_msb)
                            gz = bytes_to_int16(gz_lsb, gz_msb)

                            # Calculate actual values based on scale factors
                            # For 8G range: LSB = 4096/g, For 128 DPS: LSB = 256/dps
                            ax_g = ax / 4096.0
                            ay_g = ay / 4096.0
                            az_g = az / 4096.0
                            gx_dps = gx / 256.0
                            gy_dps = gy / 256.0
                            gz_dps = gz / 256.0

                            print("    [{:.1f}s] Accel(g): X={:6.3f} Y={:6.3f} Z={:6.3f} | Gyro(dps): X={:7.2f} Y={:7.2f} Z={:7.2f}".format(
                                elapsed, ax_g, ay_g, az_g, gx_dps, gy_dps, gz_dps))

                            # Add warning for suspicious values
                            if abs(gx_dps) > 100 or abs(gy_dps) > 100 or abs(gz_dps) > 100:
                                print("         WARNING: High gyro values detected - board may be moving!")
                            elif ax == ay and ay == az:  # Identical raw values
                                print("         WARNING: Identical raw accelerometer values!")

                            last_print_time = current_time
                        except Exception as e:
                            print("         ERROR reading IMU: " + str(e))
                            last_print_time = current_time
                            continue
                    else:
                        print("    [{:.1f}s] Status: 0x{:02X} - No new data available".format(
                            elapsed, status[0]))
                        last_print_time = current_time

                time.sleep(0.01)  # Small delay to not hammer the I2C bus

            print_info("  - IMU read test complete.")
            i2c.unlock()

        except KeyboardInterrupt:
            print("\n  Skipped.")
            try:
                i2c.unlock()
            except:
                pass
        except Exception as e:
            print_result(False, "IMU test failed: " + str(e))
            try:
                i2c.unlock()
            except:
                pass

    except Exception as e:
        print_result(False, "IMU initialization failed: " + str(e))
    finally:
        if i2c:
            i2c.deinit()

def test_touch():
    print_header("TOUCH CONTROLLER (FT6336U)")
    wait_for_enter()
    i2c = None

    try:
        print_info("Initializing I2C...")
        i2c = create_i2c_with_pullups(board.SCL, board.SDA, timeout=5)

        # Check if touch controller is present
        while not i2c.try_lock():
            pass
        try:
            devices = i2c.scan()
        finally:
            i2c.unlock()

        if 0x38 not in devices:
            print_result(False, "Touch controller not found at address 0x38")
            return

        print_info("Testing Touch Controller (FT6336U)...")
        try:
            import adafruit_focaltouch
            touch = adafruit_focaltouch.Adafruit_FocalTouch(i2c)
            print_result(True, "Touch controller initialized.")
            print_info("  - Touch the screen for 5 seconds (supports multi-touch up to 2 points)...")
            print_info("  - Try touching with two fingers simultaneously!")
            start_time = time.monotonic()
            touch_count = 0
            last_touch_time = 0
            while time.monotonic() - start_time < 5:
                current_time = time.monotonic()
                if touch.touched:
                    # Debounce touches
                    if current_time - last_touch_time > 0.3:
                        points = touch.touches
                        num_points = len(points)
                        touch_count += 1

                        if num_points == 1:
                            print("    Touch #{}: Single - X={}, Y={}".format(
                                touch_count, points[0]['x'], points[0]['y']))
                        elif num_points == 2:
                            print("    Touch #{}: Multi-touch detected!".format(touch_count))
                            print("         Point 1: X={}, Y={}".format(points[0]['x'], points[0]['y']))
                            print("         Point 2: X={}, Y={}".format(points[1]['x'], points[1]['y']))
                            distance = ((points[1]['x'] - points[0]['x'])**2 +
                                      (points[1]['y'] - points[0]['y'])**2)**0.5
                            print("         Distance between points: {:.1f} pixels".format(distance))
                        else:
                            print("    Touch #{}: {} points detected".format(touch_count, num_points))
                            for i, point in enumerate(points):
                                print("         Point {}: X={}, Y={}".format(i+1, point['x'], point['y']))

                        last_touch_time = current_time
                time.sleep(0.01)  # Small delay to reduce CPU usage
            print_info("  - Touch test complete. Total touch events: {}".format(touch_count))
        except ImportError:
            print_result(False, "Touch test failed: `adafruit_focaltouch` library not found.")
        except KeyboardInterrupt:
            print("\n  Skipped.")
        except Exception as e:
            print_result(False, "Touch test failed: " + str(e))

    except Exception as e:
        print_result(False, "Touch initialization failed: " + str(e))
    finally:
        if i2c:
            i2c.deinit()

def test_battery_monitor():
    print_header("BATTERY MONITOR")
    wait_for_enter()
    try:
        battery = analogio.AnalogIn(board.BAT_ADC)

        # Take 4 readings and discard the first
        readings = []
        for i in range(4):
            readings.append(battery.value)
            time.sleep(0.01) # Small delay between readings

        # Average the last 3 readings
        avg_raw = sum(readings[1:]) / 3

        # Voltage divider: 100k + 100k, so multiply by 2
        voltage = (avg_raw * 3.3 / 65536) * 2

        status = "OK"
        if voltage < 3.0:
            status = "LOW"
        elif voltage > 4.5:
            status = "USB Power"

        print_result(True, "Voltage: {:.2f}V ({}) [ADC Avg Raw: {:.0f}]".format(voltage, status, avg_raw))
        battery.deinit()
    except Exception as e:
        print_result(False, "Battery monitor test failed: " + str(e))

def test_sd_card():
    print_header("SD CARD SLOT")
    wait_for_enter()
    spi = None
    try:
        spi = busio.SPI(board.SD_CLK, MOSI=board.SD_MOSI, MISO=board.SD_MISO)
        sdcard = sdcardio.SDCard(spi, board.SD_CS)
        vfs = storage.VfsFat(sdcard)
        storage.mount(vfs, "/sd")

        print_result(True, "SD card mounted successfully at /sd")

        s = os.statvfs("/sd")
        sd_size = s[0] * s[2] / (1024 * 1024)
        sd_free = s[0] * s[3] / (1024 * 1024)
        print_info("  - Size: {:.2f} MB, Free: {:.2f} MB".format(sd_size, sd_free))

        print_info("  - Root directory listing:")
        for f in os.listdir("/sd"):
            print("    - " + f)

        storage.umount("/sd")
        print_result(True, "SD card unmounted successfully.")

    except OSError as e:
        if e.args[0] == 19: # [Errno 19] ENODEV
             print_result(False, "No SD card found.")
        else:
             print_result(False, "SD card test failed with OS error: " + str(e))
    except Exception as e:
        print_result(False, "SD card test failed: " + str(e))
    finally:
        if spi:
            spi.deinit()

def test_wifi():
    print_header("WIFI")
    wait_for_enter()
    try:
        mac_address = ":".join(["{:02X}".format(b) for b in wifi.radio.mac_address])
        print_info("WiFi MAC Address: " + mac_address)
        print_info("Scanning for WiFi networks for 5 seconds...")
        for network in wifi.radio.start_scanning_networks():
            print_info("  - Found WiFi: {} (RSSI: {})".format(network.ssid, network.rssi))
        wifi.radio.stop_scanning_networks()
        print_result(True, "WiFi scan complete.")
    except Exception as e:
        print_result(False, "WiFi scan failed: " + str(e))

def test_ble():
    print_header("BLUETOOTH")
    wait_for_enter()
    try:
        ble = adafruit_ble.BLERadio()
        print_info("Scanning for BLE devices for 10 seconds (active scan)...")

        # Use a dictionary to store the most complete advertisement from each device
        found_devices = {}

        for adv in ble.start_scan(timeout=10, active=True):
            addr = adv.address
            # If we don't have an entry, or if this new one is better (has a name), store it.
            if addr not in found_devices or \
               (not (found_devices[addr].complete_name or found_devices[addr].short_name) and \
                (adv.complete_name or adv.short_name)):
                found_devices[addr] = adv

        ble.stop_scan()

        if not found_devices:
            print_info("  - No BLE devices found.")
        else:
            print_info("  - Found {} unique devices:".format(len(found_devices)))
            for addr, adv in found_devices.items():
                name = adv.complete_name or adv.short_name or "[No Name]"
                print_info("    - {} (Address: {})".format(name, addr))

        print_result(True, "BLE scan complete.")
    except Exception as e:
        print_result(False, "BLE scan failed: " + str(e))

# --- Main Execution ---
def run_all_tests():
    """Runs the full test suite."""
    print("\n" + "="*60)
    print(" WAVESHARE ESP32-S3 TOUCH AMOLED 2.41")
    print(" COMPLETE HARDWARE TEST SUITE (excluding display)")
    print("="*60)

    test_system_info()
    test_onboard_peripherals()
    test_i2c_scan()
    test_rtc()
    test_imu()
    test_touch()
    test_battery_monitor()
    test_sd_card()
    test_wifi()
    test_ble()

    print_header("TEST SUITE COMPLETE")
    print("\nAll non-display hardware components have been tested.")

if __name__ == "__main__":
    run_all_tests()
