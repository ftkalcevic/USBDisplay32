# USB LCD Monitor

An LCD module that connects to linux as a mini monitor.  Currently supports 400x240 HX8352A.

## History
<table>
<tr>
<th>Version</th><th>Description</th>
</tr>
<tr>
<td>1.0</td><td>Initial version used an 8 bit Atmega128.  A simple command set was implemented in the firmware to display rectangles, lines, text.</td>
<td>2.0</td><td>Upgrade to 32 bit, AT32UC3B1256.  The same simple command set, but heaps faster on a 32bit 60Mhz chip.</td>
<td>3.0</td><td>Paradigm shift - rather having to write client software specifically for the LCD, the LCD now becomes a USB LCD monitor, so any X-Windows application, can now be displayed on LCD (assuming it looks ok on a small display).  
This was possible by upgrading to a AT32UC3A3256, to use the EBI (external bus interface) - awesome speed upgrade - and the USB 2.0 high speed interface.
</td>
</tr>
</table>

## Details

This is the initial commit to github.  It is still a work in progress.  This is a rewrite, after I wasn't able to source any more LCDs using the R61509V chipset.
