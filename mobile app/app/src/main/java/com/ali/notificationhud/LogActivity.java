package com.ali.notificationhud;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.MenuItem;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

public class LogActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_log);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        TextView logText = findViewById(R.id.logText);
        Button btnClearLogs = findViewById(R.id.btnClearLogsActivity);

        SharedPreferences prefs = getSharedPreferences("LogPrefs", MODE_PRIVATE);
        String log = prefs.getString("log", "");
        logText.setText(log);

        btnClearLogs.setOnClickListener(v -> {
            LoggerUtil.clearLogs();
            logText.setText("");
            Toast.makeText(this, "Logs cleared", Toast.LENGTH_SHORT).show();
        });
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}