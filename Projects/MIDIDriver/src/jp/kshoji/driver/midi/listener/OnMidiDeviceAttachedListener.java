package jp.kshoji.driver.midi.listener;

import android.hardware.usb.UsbDevice;

import jp.kshoji.driver.midi.device.MidiInputDevice;
import jp.kshoji.driver.midi.device.MidiOutputDevice;
import jp.kshoji.driver.midi.util.NonNull;

/**
 * Listener for MIDI attached events
 * 
 * @author K.Shoji
 */
public interface OnMidiDeviceAttachedListener {

    /**
     * device has been attached
     *
     * @param usbDevice the attached UsbDevice
     */
    @Deprecated
    void onDeviceAttached(@NonNull UsbDevice usbDevice);

    /**
     * MIDI input device has been attached
     *
     * @param midiInputDevice attached MIDI Input device
     */
    void onMidiInputDeviceAttached(@NonNull MidiInputDevice midiInputDevice);

    /**
     * MIDI output device has been attached
     *
     * @param midiOutputDevice attached MIDI Output device
     */
    void onMidiOutputDeviceAttached(@NonNull MidiOutputDevice midiOutputDevice);
}
