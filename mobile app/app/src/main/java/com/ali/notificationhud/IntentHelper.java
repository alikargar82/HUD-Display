package com.ali.notificationhud;

import android.content.Context;
import android.content.Intent;
import android.os.Build;

/**
 * Helper class for managing intents to BLEService
 */
public class IntentHelper {

    /**
     * Send a message to ESP32 via BLE
     */
    public static void sendMessageToESP32(Context context, String message) {
        Intent intent = new Intent(context, BLEService.class);
        intent.setAction(BLEService.ACTION_SEND_MESSAGE);
        intent.putExtra(BLEService.EXTRA_MESSAGE, message);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            context.startForegroundService(intent);
        } else {
            context.startService(intent);
        }
    }

    /**
     * Connect to a BLE device
     */
    public static void connectToDevice(Context context, String deviceAddress) {
        Intent intent = new Intent(context, BLEService.class);
        intent.setAction(BLEService.ACTION_CONNECT);
        intent.putExtra(BLEService.EXTRA_DEVICE_ADDRESS, deviceAddress);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            context.startForegroundService(intent);
        } else {
            context.startService(intent);
        }
    }

    /**
     * Disconnect from BLE device
     */
    public static void disconnectFromDevice(Context context) {
        Intent intent = new Intent(context, BLEService.class);
        intent.setAction(BLEService.ACTION_DISCONNECT);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            context.startForegroundService(intent);
        } else {
            context.startService(intent);
        }
    }
}

