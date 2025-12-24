package com.ali.notificationhud;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ServiceInfo;
import android.os.Binder;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;
import androidx.core.app.NotificationCompat;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.Queue;
import java.util.UUID;

public class BLEService extends Service {
    private static final String TAG = "BLEService";

    // UUIDs for ESP32 BLE service (MUST match ESP32 code!)
    private static final UUID SERVICE_UUID = UUID.fromString("12345678-1234-1234-1234-1234567890ab");
    private static final UUID CHARACTERISTIC_UUID = UUID.fromString("abcd1234-5678-90ab-cdef-1234567890ab");

    public static final String ACTION_CONNECT = "com.ali.notificationhud.ACTION_CONNECT";
    public static final String ACTION_DISCONNECT = "com.ali.notificationhud.ACTION_DISCONNECT";
    public static final String ACTION_SEND_MESSAGE = "com.ali.notificationhud.ACTION_SEND_MESSAGE";
    public static final String EXTRA_DEVICE_ADDRESS = "com.ali.notificationhud.EXTRA_DEVICE_ADDRESS";
    public static final String EXTRA_MESSAGE = "com.ali.notificationhud.EXTRA_MESSAGE";
    public static final String ACTION_GATT_CONNECTED = "com.ali.notificationhud.ACTION_GATT_CONNECTED";
    public static final String ACTION_GATT_DISCONNECTED = "com.ali.notificationhud.ACTION_GATT_DISCONNECTED";

    private static final String CHANNEL_ID = "BLEServiceChannel";
    private static final int NOTIFICATION_ID = 1;

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothGatt bluetoothGatt;
    private boolean connected = false;
    private boolean servicesDiscovered = false;
    private String deviceAddress;
    private static final int MTU_SIZE = 517;
    private static final int MAX_PAYLOAD_SIZE = 512;  // Leave room for overhead

    // Message queue to handle multiple sends
    private final Queue<String> messageQueue = new LinkedList<>();
    private boolean isWriting = false;

    private final IBinder binder = new LocalBinder();
    private final Handler handler = new Handler(Looper.getMainLooper());


    public class LocalBinder extends Binder {
        BLEService getService() {
            return BLEService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        createNotificationChannel();
        LoggerUtil.init(this);
        LoggerUtil.logServiceStarted();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            startForeground(NOTIFICATION_ID, createNotification(), ServiceInfo.FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE);
        } else {
            startForeground(NOTIFICATION_ID, createNotification());
        }

        if (intent != null) {
            final String action = intent.getAction();
            if (action != null) {
                switch (action) {
                    case ACTION_CONNECT:
                        deviceAddress = intent.getStringExtra(EXTRA_DEVICE_ADDRESS);
                        connect(deviceAddress);
                        break;
                    case ACTION_DISCONNECT:
                        disconnect();
                        break;
                    case ACTION_SEND_MESSAGE:
                        String message = intent.getStringExtra(EXTRA_MESSAGE);
                        sendData(message);
                        break;
                }
            }
        }
        return START_STICKY;
    }

    @SuppressLint("MissingPermission")
    public void connect(String address) {
        LoggerUtil.logConnectionAttempt("ESP32", address);

        if (bluetoothAdapter == null || address == null) {
            LoggerUtil.e("BluetoothAdapter not initialized or unspecified address");
            broadcastUpdate(ACTION_GATT_DISCONNECTED);
            return;
        }

        if (connected && deviceAddress != null && deviceAddress.equals(address) && bluetoothGatt != null) {
            LoggerUtil.d("Already connected to this device: " + address);
            return;
        }

        deviceAddress = address;

        try {
            final BluetoothDevice device = bluetoothAdapter.getRemoteDevice(deviceAddress);
            if (device == null) {
                LoggerUtil.e("Device not found with address: " + address);
                broadcastUpdate(ACTION_GATT_DISCONNECTED);
                return;
            }

            if (bluetoothGatt != null) {
                bluetoothGatt.close();
            }

            bluetoothGatt = device.connectGatt(this, false, gattCallback);
            LoggerUtil.d("connectGatt() called for " + address);

        } catch (Exception e) {
            LoggerUtil.e("Error during connection: " + e.getMessage(), e);
            broadcastUpdate(ACTION_GATT_DISCONNECTED);
        }
    }

    private void broadcastUpdate(final String action) {
        final Intent intent = new Intent(action);
        sendBroadcast(intent);
    }

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        @SuppressLint("MissingPermission")
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);

            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    LoggerUtil.logConnected(deviceAddress);
                    connected = true;
                    servicesDiscovered = false;
                    broadcastUpdate(ACTION_GATT_CONNECTED);
                    handler.post(() -> gatt.requestMtu(MTU_SIZE));
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    LoggerUtil.logDisconnected(deviceAddress);
                    closeGatt();
                }
            } else {
                LoggerUtil.e("Connection state change error, status: " + status);
                closeGatt();
            }
        }

        @Override
        @SuppressLint("MissingPermission")
        public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
            super.onMtuChanged(gatt, mtu, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                LoggerUtil.logMtuChanged(mtu);
            } else {
                LoggerUtil.w("MTU change failed, status: " + status);
            }
            LoggerUtil.d("Discovering GATT services...");
            gatt.discoverServices();
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);

            if (status == BluetoothGatt.GATT_SUCCESS) {
                LoggerUtil.logServicesDiscovered();
                servicesDiscovered = true;
                handler.post(() -> processQueue()); // Process any queued messages
            } else {
                LoggerUtil.w("Services discovery failed with status: " + status);
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicWrite(gatt, characteristic, status);

            if (status == BluetoothGatt.GATT_SUCCESS) {
                LoggerUtil.logMessageWriteSuccess(0);
                messageQueue.poll();
            } else {
                LoggerUtil.logMessageWriteFailed(status);
            }
            isWriting = false;
            if (!messageQueue.isEmpty()) {
                handler.postDelayed(() -> processQueue(), 50); // Process next message after a small delay
            }
        }
    };

    public void sendData(String data) {
        if (data == null || data.isEmpty()) {
            LoggerUtil.w("Empty data cannot be sent");
            return;
        }
        messageQueue.add(data);
        LoggerUtil.logMessageQueued(data, messageQueue.size());
        handler.post(() -> processQueue());
    }

    @SuppressLint("MissingPermission")
    private void processQueue() {
        if (isWriting || messageQueue.isEmpty()) {
            return;
        }

        if (!connected || bluetoothGatt == null || !servicesDiscovered) {
            LoggerUtil.logMessageWaitingForConnection();
            return;
        }

        String data = messageQueue.peek();
        if (data == null) {
            return;
        }

        try {
            BluetoothGattService service = bluetoothGatt.getService(SERVICE_UUID);
            if (service == null) {
                LoggerUtil.e("GATT Service not found! UUID: " + SERVICE_UUID);
                messageQueue.poll();
                handler.post(() -> processQueue());
                return;
            }

            BluetoothGattCharacteristic characteristic = service.getCharacteristic(CHARACTERISTIC_UUID);
            if (characteristic == null) {
                LoggerUtil.e("GATT Characteristic not found! UUID: " + CHARACTERISTIC_UUID);
                messageQueue.poll();
                handler.post(() -> processQueue());
                return;
            }

            int charProperties = characteristic.getProperties();
            boolean hasWrite = (charProperties & BluetoothGattCharacteristic.PROPERTY_WRITE) != 0;
            boolean hasWriteNoResponse = (charProperties & BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) != 0;

            if (!hasWrite && !hasWriteNoResponse) {
                LoggerUtil.e("Characteristic has no write permission!");
                messageQueue.poll();
                handler.post(() -> processQueue());
                return;
            }

            characteristic.setWriteType(hasWrite ? BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT : BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);

            byte[] dataBytes = data.getBytes(StandardCharsets.UTF_8);
            String writeType = hasWrite ? "WRITE" : "WRITE_NO_RESPONSE";

            if (dataBytes.length > MAX_PAYLOAD_SIZE) {
                LoggerUtil.w("Data size " + dataBytes.length + " exceeds MAX_PAYLOAD_SIZE " + MAX_PAYLOAD_SIZE + ", truncating");
                dataBytes = Arrays.copyOf(dataBytes, MAX_PAYLOAD_SIZE);
            }

            characteristic.setValue(dataBytes);
            LoggerUtil.logMessageWriting(data, dataBytes.length, writeType);

            isWriting = true;
            if (!bluetoothGatt.writeCharacteristic(characteristic)) {
                LoggerUtil.e("Failed to initiate write operation");
                isWriting = false;
            }

        } catch (Exception e) {
            LoggerUtil.e("Exception in processQueue", e);
            isWriting = false;
        }
    }

    @SuppressLint("MissingPermission")
    public void disconnect() {
        if (bluetoothGatt != null) {
            LoggerUtil.d("Disconnecting from BLE device...");
            bluetoothGatt.disconnect();
        } else {
            LoggerUtil.w("BluetoothGatt is null, cannot disconnect");
        }
    }

    @SuppressLint("MissingPermission")
    private void closeGatt() {
        if (bluetoothGatt != null) {
            bluetoothGatt.close();
            bluetoothGatt = null;
        }
        connected = false;
        servicesDiscovered = false;
        isWriting = false;
        messageQueue.clear();
        broadcastUpdate(ACTION_GATT_DISCONNECTED);
        LoggerUtil.d("GATT connection closed and cleaned up");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        closeGatt();
    }
    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                    CHANNEL_ID,
                    "BLE Service",
                    NotificationManager.IMPORTANCE_LOW
            );
            channel.setDescription("BLE Connection Channel");
            NotificationManager notificationManager = getSystemService(NotificationManager.class);
            if (notificationManager != null) {
                notificationManager.createNotificationChannel(channel);
            }
        }
    }

    private Notification createNotification() {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, CHANNEL_ID)
                .setContentTitle("NotificationHUD")
                .setContentText("Connected to ESP32")
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setPriority(NotificationCompat.PRIORITY_LOW)
                .setOngoing(true);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            if (checkSelfPermission(android.Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
                Log.w(TAG, "Notification permission not granted, cannot show foreground notification.");
            }
        }
        return builder.build();
    }
}
