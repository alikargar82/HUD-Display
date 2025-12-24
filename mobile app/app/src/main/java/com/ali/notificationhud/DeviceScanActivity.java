package com.ali.notificationhud;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.List;

public class DeviceScanActivity extends AppCompatActivity {
    private static final long SCAN_PERIOD = 10000; // 10 seconds

    private BluetoothLeScanner bluetoothLeScanner;
    private boolean scanning = false;
    private final Handler handler = new Handler(Looper.getMainLooper());

    private final List<BLEDevice> deviceList = new ArrayList<>();
    private DeviceListAdapter adapter;
    private Button btnScan;
    private TextView txtEmpty;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device_scan);

        initUI();
        initBluetooth();
        startScan();
    }

    private void initUI() {
        RecyclerView recyclerView = findViewById(R.id.recyclerView);
        btnScan = findViewById(R.id.btnScan);
        txtEmpty = findViewById(R.id.txtEmpty);

        Button btnBack = findViewById(R.id.btnBack);
        btnBack.setOnClickListener(v -> finish());

        recyclerView.setLayoutManager(new LinearLayoutManager(this));
        adapter = new DeviceListAdapter(deviceList, this::connectToDevice);
        recyclerView.setAdapter(adapter);

        btnScan.setOnClickListener(v -> {
            if (scanning) {
                stopScan();
            } else {
                startScan();
            }
        });
    }

    private void initBluetooth() {
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter != null) {
            bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner();
        }
    }

    @SuppressLint("MissingPermission")
    private void startScan() {
        if (bluetoothLeScanner == null) {
            Toast.makeText(this, "Bluetooth not available", Toast.LENGTH_SHORT).show();
            LoggerUtil.e("BluetoothLeScanner is null");
            return;
        }

        deviceList.clear();
        adapter.notifyDataSetChanged();
        txtEmpty.setVisibility(View.VISIBLE);

        LoggerUtil.logScanStarted();
        bluetoothLeScanner.startScan(scanCallback);
        scanning = true;
        btnScan.setText("STOP");

        handler.postDelayed(this::stopScan, SCAN_PERIOD);
        Toast.makeText(this, "Scan started...", Toast.LENGTH_SHORT).show();
    }

    @SuppressLint("MissingPermission")
    private void stopScan() {
        if (scanning && bluetoothLeScanner != null) {
            bluetoothLeScanner.stopScan(scanCallback);
            scanning = false;
            btnScan.setText("SCAN");
            LoggerUtil.logScanStopped(deviceList.size());
            if (deviceList.isEmpty()) {
                txtEmpty.setVisibility(View.VISIBLE);
            }
        }
    }

    private final ScanCallback scanCallback = new ScanCallback() {
        @SuppressLint("MissingPermission")
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            if (result == null) return;

            try {
                String deviceName = result.getDevice().getName();
                if (deviceName == null || deviceName.isEmpty()) {
                    deviceName = "Unknown Device";
                }

                if (deviceName.toLowerCase().contains("esp32") || deviceName.toLowerCase().contains("hud")) {
                    String address = result.getDevice().getAddress();
                    int rssi = result.getRssi();
                    LoggerUtil.logDeviceFound(deviceName, address, rssi);

                    BLEDevice newDevice = new BLEDevice(deviceName, address, rssi);
                    runOnUiThread(() -> {
                        adapter.addOrUpdateDevice(newDevice);
                        txtEmpty.setVisibility(View.GONE);
                    });
                }

            } catch (Exception e) {
                LoggerUtil.e("Error processing scan result", e);
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            super.onScanFailed(errorCode);
            LoggerUtil.e("BLE Scan failed with error code: " + errorCode);
            Toast.makeText(DeviceScanActivity.this, "Scan failed: " + errorCode, Toast.LENGTH_SHORT).show();
        }
    };

    private void connectToDevice(BLEDevice device) {
        LoggerUtil.logConnectionAttempt(device.getName(), device.getAddress());

        getSharedPreferences("BLEPrefs", MODE_PRIVATE)
                .edit()
                .putString("device_address", device.getAddress())
                .putString("device_name", device.getName())
                .apply();

        Toast.makeText(this, "Connecting to " + device.getName(), Toast.LENGTH_SHORT).show();

        Intent bleServiceIntent = new Intent(this, BLEService.class);
        bleServiceIntent.setAction(BLEService.ACTION_CONNECT);
        bleServiceIntent.putExtra(BLEService.EXTRA_DEVICE_ADDRESS, device.getAddress());

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(bleServiceIntent);
        } else {
            startService(bleServiceIntent);
        }
        finish(); // Return to main activity
    }

    @Override
    protected void onDestroy() {
        stopScan();
        super.onDestroy();
    }
}