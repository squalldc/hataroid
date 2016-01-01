package com.RetroSoft.Hataroid.Midi;

import android.content.Context;
import android.hardware.usb.UsbDevice;

import com.RetroSoft.Hataroid.HataroidNativeLib;

import java.util.Set;

import jp.kshoji.driver.midi.device.MidiInputDevice;
import jp.kshoji.driver.midi.device.MidiOutputDevice;
import jp.kshoji.driver.midi.util.NonNull;
import jp.kshoji.driver.midi.util.UsbMidiDriver;

public class USBMidi
{
	private USBMidiInterface _usbMidiInterface;

	public USBMidi()
	{
	}

	public void init(Context appContext)
	{
		_usbMidiInterface = new USBMidiInterface(appContext);
		_usbMidiInterface.open();
	}

	public void deinit()
	{
		if (_usbMidiInterface != null)
		{
			_usbMidiInterface.close();
		}
	}

	public void sendMidiByte(byte b)
	{
		if (_usbMidiInterface != null)
		{
			_usbMidiInterface.sendMidiByte(b);
		}
	}

	public void sendMidiBytes(byte[] b)
	{
		if (_usbMidiInterface != null)
		{
			_usbMidiInterface.sendMidiBytes(b);
		}
	}
}

class USBMidiInterface extends UsbMidiDriver
{
	byte[] _recvBuf = new byte[4];

	public USBMidiInterface(Context appContext)
	{
		super(appContext);
	}

	@Deprecated
	@Override
	public void onDeviceAttached(@NonNull UsbDevice usbDevice)
	{
	}

	@Deprecated
	@Override
	public void onDeviceDetached(@NonNull UsbDevice usbDevice)
	{
	}

	@Override
	public void onMidiInputDeviceAttached(@NonNull MidiInputDevice midiInputDevice)
	{
	}

	@Override
	public void onMidiOutputDeviceAttached(@NonNull final MidiOutputDevice midiOutputDevice)
	{
		//Log.i("hataroid", "+MIDI Output Device Attached: " + midiOutputDevice.getUsbDevice().getDeviceName());
	}

	@Override
	public void onMidiInputDeviceDetached(@NonNull MidiInputDevice midiInputDevice)
	{
	}

	@Override
	public void onMidiOutputDeviceDetached(@NonNull final MidiOutputDevice midiOutputDevice)
	{
		//Log.i("hataroid", "-MIDI Output Device Removed: " + midiOutputDevice.getUsbDevice().getDeviceName());
	}

	@Override
	public void onMidiNoteOff(@NonNull final MidiInputDevice sender, int cable, int channel, int note, int velocity)
	{
	}

	@Override
	public void onMidiNoteOn(@NonNull final MidiInputDevice sender, int cable, int channel, int note, int velocity)
	{
	}

	@Override
	public void onMidiPolyphonicAftertouch(@NonNull final MidiInputDevice sender, int cable, int channel, int note, int pressure)
	{
	}

	@Override
	public void onMidiControlChange(@NonNull final MidiInputDevice sender, int cable, int channel, int function, int value)
	{
	}

	@Override
	public void onMidiProgramChange(@NonNull final MidiInputDevice sender, int cable, int channel, int program)
	{
	}

	@Override
	public void onMidiChannelAftertouch(@NonNull final MidiInputDevice sender, int cable, int channel, int pressure)
	{
	}

	@Override
	public void onMidiPitchWheel(@NonNull final MidiInputDevice sender, int cable, int channel, int amount)
	{
	}

	@Override
	public void onMidiSystemExclusive(@NonNull final MidiInputDevice sender, int cable, final byte[] systemExclusive)
	{
	}

	@Override
	public void onMidiSystemCommonMessage(@NonNull final MidiInputDevice sender, int cable, final byte[] bytes)
	{
	}

	@Override
	public void onMidiSingleByte(@NonNull final MidiInputDevice sender, int cable, int byte1)
	{
	}

	@Override
	public void onMidiMiscellaneousFunctionCodes(@NonNull final MidiInputDevice sender, int cable, int byte1, int byte2, int byte3)
	{
	}

	@Override
	public void onMidiCableEvents(@NonNull final MidiInputDevice sender, int cable, int byte1, int byte2, int byte3)
	{
	}

	@Override
	public void onMidiTimeCodeQuarterFrame(@NonNull MidiInputDevice sender, int cable, int timing) {
	}

	@Override
	public void onMidiSongSelect(@NonNull MidiInputDevice sender, int cable, int song) {
	}

	@Override
	public void onMidiSongPositionPointer(@NonNull MidiInputDevice sender, int cable, int position) {
	}

	@Override
	public void onMidiTuneRequest(@NonNull MidiInputDevice sender, int cable) {
	}

	@Override
	public void onMidiTimingClock(@NonNull MidiInputDevice sender, int cable) {
	}

	@Override
	public void onMidiStart(@NonNull MidiInputDevice sender, int cable) {
	}

	@Override
	public void onMidiContinue(@NonNull MidiInputDevice sender, int cable) {
	}

	@Override
	public void onMidiStop(@NonNull MidiInputDevice sender, int cable) {
	}

	@Override
	public void onMidiActiveSensing(@NonNull MidiInputDevice sender, int cable) {
	}

	@Override
	public void onMidiReset(@NonNull MidiInputDevice sender, int cable) {
	}

	@Override
	public void onMidiRawBytes1(byte b1)
	{
		_recvBuf[0] = b1;
		HataroidNativeLib.emulatorReceiveHardwareMidiBytes(1, _recvBuf);
	}

	@Override
	public void onMidiRawBytes2(byte b1, byte b2)
	{
		_recvBuf[0] = b1;
		_recvBuf[1] = b2;
		HataroidNativeLib.emulatorReceiveHardwareMidiBytes(2, _recvBuf);
	}

	@Override
	public void onMidiRawBytes3(byte b1, byte b2, byte b3)
	{
		_recvBuf[0] = b1;
		_recvBuf[1] = b2;
		_recvBuf[2] = b3;
		HataroidNativeLib.emulatorReceiveHardwareMidiBytes(3, _recvBuf);
	}

	public void sendMidiByte(int b)
	{
		Set<MidiOutputDevice> midiOutputDevices = getMidiOutputDevices();

		if (midiOutputDevices.size() > 0)
		{
			// returns the first one.
			MidiOutputDevice mdev = (MidiOutputDevice) midiOutputDevices.toArray()[0];
			int cable = 0; // only support one device for now
			mdev.sendMidiSingleByte(cable, b);
		}
	}

	public void sendMidiBytes(byte[] bytes)
	{
		Set<MidiOutputDevice> midiOutputDevices = getMidiOutputDevices();

		if (midiOutputDevices.size() > 0)
		{
			// returns the first one.
			MidiOutputDevice mdev = (MidiOutputDevice) midiOutputDevices.toArray()[0];
			int cable = 0; // only support one device for now
			for (int i = 0; i < bytes.length; ++i)
			{
				mdev.sendMidiSingleByte(cable, bytes[i]);
			}
		}
	}
}
