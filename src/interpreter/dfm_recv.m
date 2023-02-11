%% collection
close all
clear

    spla = "COM8";
    s = serialport(spla, 115200);

%%
while 1
    while 1
       c = read(s, 1, "uint8");
       if c == 35 % start of a packet
           break
       end
    end
        packetnum = read(s, 1, "uint32")
        id = read(s, 1, "uint16")
        cons = read(s, 1, "uint16")
        status = read(s, 1, "uint16")
        sw = read(s, 1, "uint16")
        bat = read(s, 1, "uint32")
        freq = read(s, 1, "int32")
        uptime = read(s, 1, "uint32")
        toa = read(s, 1, "uint32")
        temp = read(s, 1, "single")
        acx = read(s, 1, "single")
        acy = read(s, 1, "single")
        acz = read(s, 1, "single")
        epoch = read(s, 1, "uint32")
        newl = read(s, 2, "uint8")

end

