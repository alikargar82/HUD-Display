package com.ali.notificationhud;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.List;

public class FilteredAppsAdapter extends RecyclerView.Adapter<FilteredAppsAdapter.ViewHolder> {
    private final List<FilterAppsActivity.AppInfo> appList;
    private final OnAppToggleListener listener;

    public interface OnAppToggleListener {
        void onToggle(FilterAppsActivity.AppInfo app, boolean isFiltered);
    }

    public FilteredAppsAdapter(List<FilterAppsActivity.AppInfo> appList, OnAppToggleListener listener) {
        this.appList = appList;
        this.listener = listener;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_filtered_app, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        FilterAppsActivity.AppInfo app = appList.get(position);
        holder.appName.setText(app.appName);
        holder.checkbox.setChecked(app.isFiltered);
        holder.checkbox.setOnCheckedChangeListener((buttonView, isChecked) -> {
            app.isFiltered = isChecked;
            listener.onToggle(app, isChecked);
        });
    }

    @Override
    public int getItemCount() {
        return appList.size();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        public TextView appName;
        public CheckBox checkbox;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            appName = itemView.findViewById(R.id.appName);
            checkbox = itemView.findViewById(R.id.checkbox);
        }
    }
}

