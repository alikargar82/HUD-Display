package com.ali.notificationhud;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.List;

public class DeviceListAdapter extends RecyclerView.Adapter<DeviceListAdapter.ViewHolder> {

    public interface OnDeviceClickListener {
        void onDeviceClick(BLEDevice device);
    }

    private List<BLEDevice> deviceList;
    private OnDeviceClickListener listener;

    public DeviceListAdapter(List<BLEDevice> deviceList, OnDeviceClickListener listener) {
        this.deviceList = deviceList;
        this.listener = listener;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.item_device, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        BLEDevice device = deviceList.get(position);
        holder.deviceName.setText(device.getName());
        holder.deviceAddress.setText(device.getAddress());
        holder.deviceRssi.setText("RSSI: " + device.getRssi() + " dBm");

        holder.itemView.setOnClickListener(v -> {
            if (listener != null) {
                listener.onDeviceClick(device);
            }
        });
    }

    @Override
    public int getItemCount() {
        return deviceList.size();
    }

    public void addOrUpdateDevice(BLEDevice newDevice) {
        for (int i = 0; i < deviceList.size(); i++) {
            if (deviceList.get(i).getAddress().equals(newDevice.getAddress())) {
                deviceList.get(i).setRssi(newDevice.getRssi());
                notifyItemChanged(i);
                return;
            }
        }
        deviceList.add(newDevice);
        notifyItemInserted(deviceList.size() - 1);
    }

    static class ViewHolder extends RecyclerView.ViewHolder {
        TextView deviceName, deviceAddress, deviceRssi;

        ViewHolder(View itemView) {
            super(itemView);
            deviceName = itemView.findViewById(R.id.deviceName);
            deviceAddress = itemView.findViewById(R.id.deviceAddress);
            deviceRssi = itemView.findViewById(R.id.deviceRssi);
        }
    }
}