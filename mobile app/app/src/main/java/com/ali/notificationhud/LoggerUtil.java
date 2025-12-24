package com.ali.notificationhud;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * Centralized logging utility for the entire application
 * Logs to both SharedPreferences (for LogActivity display) and Logcat (for debug)
 */
public class LoggerUtil {
    private static final String TAG = "NotificationHUD";
    private static final String PREFS_NAME = "LogPrefs";
    private static final String LOG_KEY = "log";

    private static Context appContext;

    /**
     * Initialize the logger with application context
     */
    public static void init(Context context) {
        appContext = context.getApplicationContext();
    }

    /**
     * Log a general message
     */
    public static void d(String message) {
        log("DEBUG", message);
    }

    /**
     * Log an info message
     */
    public static void i(String message) {
        log("INFO", message);
    }

    /**
     * Log a warning message
     */
    public static void w(String message) {
        log("WARN", message);
    }

    /**
     * Log an error message
     */
    public static void e(String message) {
        log("ERROR", message);
    }

    /**
     * Log an error message with exception
     */
    public static void e(String message, Throwable e) {
        log("ERROR", message + " - " + e.getMessage());
    }

    /**
     * Log scan lifecycle events
     */
    public static void logScanStarted() {
        log("SCAN", "🔍 BLE Scan STARTED");
    }

    public static void logScanStopped(int deviceCount) {
        log("SCAN", "🔍 BLE Scan STOPPED - Found " + deviceCount + " device(s)");
    }

    public static void logDeviceFound(String name, String address, int rssi) {
        log("SCAN", "✓ Device found: " + name + " [" + address + "] RSSI: " + rssi + " dBm");
    }

    /**
     * Log connection lifecycle events
     */
    public static void logConnectionAttempt(String deviceName, String address) {
        log("CONNECT", "🔗 Connecting to " + deviceName + " [" + address + "]");
    }

    public static void logConnected(String deviceName) {
        log("CONNECT", "✅ Connected to " + deviceName);
    }

    public static void logConnectionFailed(String reason) {
        log("CONNECT", "❌ Connection failed: " + reason);
    }

    public static void logDisconnected(String deviceName) {
        log("CONNECT", "🔌 Disconnected from " + deviceName);
    }

    public static void logServicesDiscovered() {
        log("CONNECT", "✓ GATT Services discovered successfully");
    }

    public static void logMtuChanged(int mtu) {
        log("CONNECT", "✓ MTU changed to " + mtu + " bytes");
    }

    /**
     * Log permission events
     */
    public static void logPermissionRequested(String permissionName) {
        log("PERMISSION", "⚠️ Requesting permission: " + permissionName);
    }

    public static void logPermissionGranted(String permissionName) {
        log("PERMISSION", "✅ Permission granted: " + permissionName);
    }

    public static void logPermissionDenied(String permissionName) {
        log("PERMISSION", "❌ Permission denied: " + permissionName);
    }

    /**
     * Log notification events
     */
    public static void logNotificationReceived(String appName, String title, String text) {
        log("NOTIFICATION", "📨 Notification received from: " + appName);
        log("NOTIFICATION", "    Title: " + title);
        log("NOTIFICATION", "    Text: " + text);
    }

    public static void logNotificationParsed(String formattedData) {
        log("NOTIFICATION", "✓ Parsed notification: " + formattedData);
    }

    public static void logNotificationDropped(String reason) {
        log("NOTIFICATION", "⚠️ Notification dropped: " + reason);
    }

    /**
     * Log message transmission events
     */
    public static void logMessageQueued(String message, int queueSize) {
        log("MESSAGE", "📤 Message queued [Queue size: " + queueSize + "]: " + truncateMessage(message, 50));
    }

    public static void logMessageWriting(String message, int dataSize, String writeType) {
        log("MESSAGE", "📝 Writing message [" + dataSize + " bytes, " + writeType + "]: " + truncateMessage(message, 50));
    }

    public static void logMessageWriteSuccess(int bytesWritten) {
        log("MESSAGE", "✓ Message written successfully [" + bytesWritten + " bytes sent]");
    }

    public static void logMessageWriteFailed(int errorCode) {
        log("MESSAGE", "❌ Message write failed [Error code: " + errorCode + "]");
    }

    public static void logMessageWaitingForConnection() {
        log("MESSAGE", "⏳ Message waiting for connection...");
    }

    /**
     * Log service events
     */
    public static void logServiceStarted() {
        log("SERVICE", "✓ BLE Service started");
    }

    public static void logServiceBound() {
        log("SERVICE", "✓ BLE Service bound");
    }

    /**
     * Log notification listener events
     */
    public static void logNotificationListenerBound() {
        log("NOTIF_LISTENER", "✅ NotificationListener bound to BLEService");
    }

    public static void logNotificationListenerUnbound() {
        log("NOTIF_LISTENER", "❌ NotificationListener unbound from BLEService");
    }

    /**
     * Core logging method
     */
    private static void log(String prefix, String message) {
        String time = new SimpleDateFormat("HH:mm:ss.SSS", Locale.getDefault()).format(new Date());
        String logEntry = "[" + time + "] [" + prefix + "] " + message;

        // Log to Logcat
        Log.d(TAG, logEntry);

        // Log to SharedPreferences if context is available
        if (appContext != null) {
            try {
                SharedPreferences prefs = appContext.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE);
                String currentLog = prefs.getString(LOG_KEY, "");
                String newLog = currentLog + logEntry + "\n";
                prefs.edit().putString(LOG_KEY, newLog).apply();
            } catch (Exception e) {
                Log.e(TAG, "Error writing to SharedPreferences", e);
            }
        }
    }

    /**
     * Helper to truncate long messages for display
     */
    private static String truncateMessage(String message, int maxLength) {
        if (message == null) return "null";
        if (message.length() > maxLength) {
            return message.substring(0, maxLength) + "...";
        }
        return message;
    }

    /**
     * Clear all logs
     */
    public static void clearLogs() {
        if (appContext != null) {
            try {
                SharedPreferences prefs = appContext.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE);
                prefs.edit().putString(LOG_KEY, "").apply();
                Log.d(TAG, "Logs cleared");
            } catch (Exception e) {
                Log.e(TAG, "Error clearing logs", e);
            }
        }
    }
}

