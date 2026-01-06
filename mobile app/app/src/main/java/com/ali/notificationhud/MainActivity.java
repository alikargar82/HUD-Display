package com.ali.notificationhud;

import android.Manifest;
import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.provider.Settings;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;

import com.google.android.material.card.MaterialCardView;
import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.switchmaterial.SwitchMaterial;

public class MainActivity extends AppCompatActivity {
    private static final int REQUEST_BLUETOOTH_PERMISSIONS = 1;

    // UI Elements
    private TextView statusText;
    private TextInputEditText editText;
    private MaterialCardView sendTestMessageCard;
    private MaterialCardView disconnectCard;
    private SwitchMaterial mirrorModeToggle;
    private Button btnDisconnect;
    private TextView locationStatusText;

    // Bluetooth
    private BluetoothAdapter bluetoothAdapter;
    private SharedPreferences prefs;
    private BroadcastReceiver bleStatusReceiver;
    private BLEService bleService;
    private boolean isBound = false;
    private boolean mirrorModeEnabled = false;

    // Activity Result Launcher for Bluetooth enable
    private ActivityResultLauncher<Intent> bluetoothEnableLauncher;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Initialize ActivityResultLauncher for Bluetooth enable
        bluetoothEnableLauncher = registerForActivityResult(
                new ActivityResultContracts.StartActivityForResult(),
                result -> {
                    if (result.getResultCode() == RESULT_OK) {
                        LoggerUtil.d("Bluetooth enabled by user");
                    } else {
                        LoggerUtil.w("User did not enable Bluetooth");
                    }
                });

        // Initialize Logger FIRST
        LoggerUtil.init(this);
        LoggerUtil.d("MainActivity created");

        // Initialize SharedPreferences
        prefs = getSharedPreferences("BLEPrefs", MODE_PRIVATE);

        // Initialize UI
        initUI();

        // Initialize Bluetooth
        initBluetooth();

        // Check permissions
        checkPermissions();

        // Update UI
        updateStatus();

        Intent serviceIntent = new Intent(this, BLEService.class);
        bindService(serviceIntent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    private void initUI() {
        statusText = findViewById(R.id.statusText);
        locationStatusText = findViewById(R.id.locationStatusText);
        sendTestMessageCard = findViewById(R.id.sendTestMessageCard);
        disconnectCard = findViewById(R.id.disconnectCard);
        mirrorModeToggle = findViewById(R.id.mirrorModeToggle);
        btnDisconnect = findViewById(R.id.btnDisconnect);

        Button btnEnableNotification = findViewById(R.id.btnEnableNotification);
        Button btnScanBLE = findViewById(R.id.btnScanBLE);
        Button btnSend = findViewById(R.id.btnSend);
        Button btnViewLog = findViewById(R.id.btnViewLog);
        Button btnFilterApps = findViewById(R.id.btnFilterApps);
        Button btnClearLogs = findViewById(R.id.btnClearLogs);
        editText = findViewById(R.id.editText);

        // Load mirror mode state
        mirrorModeEnabled = prefs.getBoolean("mirror_mode_enabled", false);
        mirrorModeToggle.setChecked(mirrorModeEnabled);

        // Button listeners
        btnEnableNotification.setOnClickListener(v -> enableNotificationAccess());
        btnScanBLE.setOnClickListener(v -> scanForDevices());
        btnSend.setOnClickListener(v -> sendTestMessage());
        btnViewLog.setOnClickListener(v -> startActivity(new Intent(this, LogActivity.class)));
        btnFilterApps.setOnClickListener(v -> startActivity(new Intent(this, FilterAppsActivity.class)));
        btnClearLogs.setOnClickListener(v -> clearAllLogs());
        btnDisconnect.setOnClickListener(v -> disconnectFromDevice());
        mirrorModeToggle.setOnCheckedChangeListener((buttonView, isChecked) -> onMirrorModeToggled(isChecked));

        bleStatusReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if (intent != null) {
                    updateStatus();
                }
            }
        };

        // Check if service is running
        checkServiceStatus();
    }

    private void onMirrorModeToggled(boolean isChecked) {
        mirrorModeEnabled = isChecked;
        prefs.edit().putBoolean("mirror_mode_enabled", isChecked).apply();

        String message = isChecked ? "|||enable-mirror-mode|||" : "|||disable-mirror-mode|||";
        if (isBound && bleService != null) {
            LoggerUtil.d("Sending mirror mode message: " + message);
            bleService.sendData(message);
        } else {
            LoggerUtil.w("BLE Service not bound, cannot send mirror mode message");
        }
    }

    private void initBluetooth() {
        BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager != null) {
            bluetoothAdapter = bluetoothManager.getAdapter();
        }
        if (prefs == null) {
            prefs = getSharedPreferences("BLEPrefs", MODE_PRIVATE);
        }
    }

    private void checkPermissions() {
        LoggerUtil.d("Checking all required permissions...");

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            // Android 12+
            String[] basePermissions = {
                    Manifest.permission.BLUETOOTH_SCAN,
                    Manifest.permission.BLUETOOTH_CONNECT,
                    Manifest.permission.ACCESS_FINE_LOCATION
            };

            java.util.List<String> permissionList = new java.util.ArrayList<>(java.util.Arrays.asList(basePermissions));

            // Add POST_NOTIFICATIONS for Android 13+
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                permissionList.add(Manifest.permission.POST_NOTIFICATIONS);
            }

            String[] allPermissions = permissionList.toArray(new String[0]);

            boolean allGranted = true;
            for (String permission : allPermissions) {
                if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    LoggerUtil.logPermissionRequested(permission);
                    break;
                }
            }

            if (!allGranted) {
                LoggerUtil.d("Not all permissions granted, requesting from user");
                ActivityCompat.requestPermissions(this, allPermissions, REQUEST_BLUETOOTH_PERMISSIONS);
            } else {
                LoggerUtil.d("All permissions already granted");
            }
        } else {
            // Android 11 and below
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                LoggerUtil.logPermissionRequested(Manifest.permission.ACCESS_FINE_LOCATION);
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, REQUEST_BLUETOOTH_PERMISSIONS);
            } else {
                LoggerUtil.d("Location permission already granted");
            }
        }
    }

    private void checkServiceStatus() {
        boolean isNotificationAccessEnabled = isNotificationServiceEnabled();
        String status = "Status:\n";

        if (isNotificationAccessEnabled) {
            status += "• ✅ Notification access enabled\n";
        } else {
            status += "• ❌ Notification access required\n";
        }

        boolean hasLocationPermission = ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED;
        if (hasLocationPermission) {
            locationStatusText.setText("📍 Location Permission: ✅ Granted");
        } else {
            locationStatusText.setText("📍 Location Permission: ❌ Not Granted");
        }

        if (bluetoothAdapter != null && bluetoothAdapter.isEnabled()) {
            status += "• ✅ Bluetooth enabled\n";
        } else {
            status += "• ❌ Bluetooth disabled\n";
        }

        if (prefs != null) {
            String deviceName = prefs.getString("device_name", null);
            if (deviceName != null) {
                status += "• 🔗 Connected to: " + deviceName + "\n";
                sendTestMessageCard.setVisibility(View.VISIBLE);
                disconnectCard.setVisibility(View.VISIBLE);
            } else {
                status += "• 🔌 No device connected\n";
                sendTestMessageCard.setVisibility(View.GONE);
                disconnectCard.setVisibility(View.GONE);
            }
        } else {
            status += "• 🔌 No device connected\n";
            sendTestMessageCard.setVisibility(View.GONE);
            disconnectCard.setVisibility(View.GONE);
        }

        statusText.setText(status);
    }

    private boolean isNotificationServiceEnabled() {
        String enabledNotificationListeners = Settings.Secure.getString(
                getContentResolver(),
                "enabled_notification_listeners"
        );
        String packageName = getPackageName();
        return enabledNotificationListeners != null && enabledNotificationListeners.contains(packageName);
    }

    private void enableNotificationAccess() {
        Intent intent = new Intent(Settings.ACTION_NOTIFICATION_LISTENER_SETTINGS);
        startActivity(intent);
        log("Opening notification access settings...");
        Toast.makeText(this, "Please find and enable notification access for NotificationHUD", Toast.LENGTH_LONG).show();
    }

    private void scanForDevices() {
        if (bluetoothAdapter == null) {
            LoggerUtil.e("Bluetooth not supported on this device");
            Toast.makeText(this, "Bluetooth not supported", Toast.LENGTH_SHORT).show();
            return;
        }

        if (!bluetoothAdapter.isEnabled()) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                    LoggerUtil.logPermissionRequested(Manifest.permission.BLUETOOTH_CONNECT);
                    ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.BLUETOOTH_CONNECT}, REQUEST_BLUETOOTH_PERMISSIONS);
                    return;
                }
            }
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            bluetoothEnableLauncher.launch(enableBtIntent);
            return;
        }

        // Start device scanning
        LoggerUtil.logScanStarted();
        Intent intent = new Intent(this, DeviceScanActivity.class);
        startActivity(intent);
    }

    private void sendTestMessage() {
        CharSequence textChar = editText.getText();
        if (textChar == null || textChar.toString().isEmpty()) {
            Toast.makeText(this, "Please enter some text", Toast.LENGTH_SHORT).show();
            LoggerUtil.w("Empty test message attempt");
            return;
        }

        String text = textChar.toString();
        if (isBound && bleService != null) {
            LoggerUtil.logMessageQueued(text, 1);
            bleService.sendData(text);
        } else {
            LoggerUtil.w("BLE Service not bound, cannot send message");
            Toast.makeText(this, "BLE Service not available", Toast.LENGTH_SHORT).show();
        }
    }

    private void disconnectFromDevice() {
        if (isBound && bleService != null) {
            bleService.disconnect();
            if (prefs != null) {
                prefs.edit().remove("device_address").remove("device_name").apply();
            }
            updateStatus();
            Toast.makeText(this, "Disconnected from ESP32", Toast.LENGTH_SHORT).show();
        }
    }

    private void clearAllLogs() {
        LoggerUtil.clearLogs();
        Toast.makeText(this, "Logs cleared", Toast.LENGTH_SHORT).show();
        updateStatus();
    }

    private void updateStatus() {
        checkServiceStatus();
    }

    private void log(String message) {
        LoggerUtil.d(message);
    }

    
    @Override
    @SuppressLint("UnspecifiedRegisterReceiverFlag")
    protected void onResume() {
        super.onResume();
        updateStatus();

        IntentFilter bleFilter = new IntentFilter();
        bleFilter.addAction(BLEService.ACTION_GATT_CONNECTED);
        bleFilter.addAction(BLEService.ACTION_GATT_DISCONNECTED);

        // Register receiver with appropriate flags based on API level
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            registerReceiver(bleStatusReceiver, bleFilter, Context.RECEIVER_NOT_EXPORTED);
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            registerReceiver(bleStatusReceiver, bleFilter, Context.RECEIVER_NOT_EXPORTED);
        } else {
            registerReceiver(bleStatusReceiver, bleFilter);
        }

        if (prefs != null) {
            String savedDevice = prefs.getString("device_address", null);
            if (savedDevice != null && !savedDevice.isEmpty()) {
                if (isBound && bleService != null) {
                    bleService.connect(savedDevice);
                }
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver(bleStatusReceiver);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (isBound && bleService != null) {
            bleService.disconnect();
        }

        if (isBound) {
            unbindService(serviceConnection);
            isBound = false;
        }

        LoggerUtil.clearLogs();
        LoggerUtil.d("MainActivity destroyed");
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_BLUETOOTH_PERMISSIONS) {
            boolean allGranted = true;
            for (int i = 0; i < grantResults.length; i++) {
                if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    LoggerUtil.logPermissionDenied(permissions[i]);
                } else {
                    LoggerUtil.logPermissionGranted(permissions[i]);
                }
            }
            if (allGranted) {
                LoggerUtil.d("✓ All permissions granted");
            } else {
                LoggerUtil.e("Some permissions were denied");
                Toast.makeText(this, "Permissions are required for BLE scanning", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private final ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            BLEService.LocalBinder binder = (BLEService.LocalBinder) service;
            bleService = binder.getService();
            isBound = true;

            if (prefs != null) {
                String savedDevice = prefs.getString("device_address", null);
                if (savedDevice != null && !savedDevice.isEmpty()) {
                    bleService.connect(savedDevice);
                }
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            bleService = null;
            isBound = false;
        }
    };
}
