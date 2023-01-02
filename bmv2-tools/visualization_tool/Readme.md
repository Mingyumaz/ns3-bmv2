# Functions needed for data visualization

* Ability to visualize the data stream in `csv` format.

* Whether to use `.pcap` format files.
    * Because the data header has a custom format, traditional data header analysis software is not applicable.
    * Use python to analyze the intercepted packages.
    * Need to correspond to the header settings in the p4 file.
    * Check for code or designs with similar functionality in the open source code.

* From the ns3 packet, recording data to a file instead of `capturing network traffic`(`.pcap` file).
