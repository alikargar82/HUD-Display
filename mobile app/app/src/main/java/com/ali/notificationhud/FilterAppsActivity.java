package com.ali.notificationhud;

import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.Button;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class FilterAppsActivity extends AppCompatActivity {
    private SharedPreferences prefs;
    private FilteredAppsAdapter adapter;
    private final List<AppInfo> appList = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_filter_apps);

        prefs = getSharedPreferences("FilteredApps", MODE_PRIVATE);
        initUI();
        loadInstalledApps();
    }

    private void initUI() {
        RecyclerView recyclerView = findViewById(R.id.recyclerViewApps);
        Button btnBack = findViewById(R.id.btnBackFilterApps);

        recyclerView.setLayoutManager(new LinearLayoutManager(this));
        adapter = new FilteredAppsAdapter(appList, this::onAppToggle);
        recyclerView.setAdapter(adapter);

        btnBack.setOnClickListener(v -> finish());
    }

    private void loadInstalledApps() {
        PackageManager packageManager = getPackageManager();
        List<ApplicationInfo> packages = packageManager.getInstalledApplications(PackageManager.GET_META_DATA);
        Set<String> filteredApps = prefs.getStringSet("filtered_packages", new HashSet<>());

        appList.clear();
        for (ApplicationInfo appInfo : packages) {
            if ((appInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0) {
                String appName = packageManager.getApplicationLabel(appInfo).toString();
                String packageName = appInfo.packageName;
                boolean isFiltered = filteredApps.contains(packageName);
                appList.add(new AppInfo(appName, packageName, isFiltered));
            }
        }

        adapter.notifyDataSetChanged();
    }

    private void onAppToggle(AppInfo app, boolean isFiltered) {
        Set<String> filteredApps = new HashSet<>(prefs.getStringSet("filtered_packages", new HashSet<>()));

        if (isFiltered) {
            filteredApps.add(app.packageName);
        } else {
            filteredApps.remove(app.packageName);
        }

        prefs.edit().putStringSet("filtered_packages", filteredApps).apply();
        LoggerUtil.d((isFiltered ? "Filtered" : "Unfiltered") + " app: " + app.appName);
    }

    public static class AppInfo {
        public String appName;
        public String packageName;
        public boolean isFiltered;

        public AppInfo(String appName, String packageName, boolean isFiltered) {
            this.appName = appName;
            this.packageName = packageName;
            this.isFiltered = isFiltered;
        }
    }
}

