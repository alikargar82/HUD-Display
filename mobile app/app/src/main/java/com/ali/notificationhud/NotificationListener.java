package com.ali.notificationhud;

import android.app.Notification;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;

public class NotificationListener extends NotificationListenerService {
    private BLEService bleService;
    private boolean isBound = false;

    private final ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            BLEService.LocalBinder binder = (BLEService.LocalBinder) service;
            bleService = binder.getService();
            isBound = true;
            LoggerUtil.logNotificationListenerBound();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            bleService = null;
            isBound = false;
            LoggerUtil.logNotificationListenerUnbound();
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        LoggerUtil.init(this);
        LoggerUtil.d("NotificationListener service created");
        Intent intent = new Intent(this, BLEService.class);
        bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (isBound) {
            unbindService(serviceConnection);
            isBound = false;
        }
        LoggerUtil.d("NotificationListener service destroyed");
    }

    @Override
    public void onNotificationPosted(StatusBarNotification sbn) {
        if (sbn == null) return;

        if (!isBound || bleService == null) {
            LoggerUtil.logNotificationDropped("BLE service not bound");
            return;
        }

        Bundle extras = sbn.getNotification().extras;
        if (extras == null) {
            LoggerUtil.logNotificationDropped("Notification extras is null");
            return;
        }

        String title = cleanText(extras.getString(Notification.EXTRA_TITLE, ""));
        String text = cleanText(extras.getString(Notification.EXTRA_TEXT, ""));

        if (title.isEmpty() && text.isEmpty()) {
            LoggerUtil.logNotificationDropped("Empty title and text");
            return;
        }

        String appName = getAppDisplayName(sbn.getPackageName());
        String data = formatForESP32(appName, title, text);

        LoggerUtil.logNotificationReceived(appName, title, text);
        LoggerUtil.logNotificationParsed(data);

        bleService.sendData(data);
        LoggerUtil.d("✓ Notification queued for sending to ESP32");
    }

    private String cleanText(String text) {
        if (text == null) return "";
        return text.replace("\n", " ")
                .replace("\r", " ")
                .replace("|", "-")
                .replace(";", ",")
                .trim();
    }

    private String getAppDisplayName(String packageName) {
        try {
            return getPackageManager().getApplicationLabel(getPackageManager().getApplicationInfo(packageName, 0)).toString();
        } catch (Exception e) {
            return packageName;
        }
    }

    private String formatForESP32(String appName, String title, String text) {
        return appName + "|" + title + "|" + text;
    }
}
