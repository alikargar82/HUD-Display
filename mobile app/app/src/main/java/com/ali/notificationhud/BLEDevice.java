package com.ali.notificationhud;

public class BLEDevice {
    private String name;
    private String address;
    private int rssi;

    public BLEDevice(String name, String address, int rssi) {
        this.name = name;
        this.address = address;
        this.rssi = rssi;
    }

    public String getName() {
        return name != null ? name : "Unknown Device";
    }

    public String getAddress() {
        return address;
    }

    public int getRssi() {
        return rssi;
    }

    public void setRssi(int rssi) {
        this.rssi = rssi;
    }
}