const TEST = false;
const SERIAL = "/dev/wchusb";
const BAUD = 9600;
const LOG_RAW_DATA = false;
const LOG_OUTGOING = true;
const LOG_INCOMING = true;
const LOG_INCOMING_STR = false;

var slip = require('node-slip');
var serial = require('serialport');
var oscmin = require('osc-min');

var receiver = {
    data: function (input) {
        if (LOG_INCOMING_STR) {
            console.log(input.toString());
        }
        read(input);
    },
    framing: function (input) {
        console.log("SLIP - Framing Error");
    },
    escape: function (input) {
        console.log("SLIP - Escape Error");
    }
};

var parser = new slip.parser(receiver);

function send(buf) {
    if (LOG_OUTGOING) {
        console.log('Outgoing > ');
        console.log(buf);
    }

    buf = oscmin.toBuffer(buf);
    buf = slip.generator(buf);
    if (LOG_RAW_DATA) {
        console.log('o', buf.toString('hex'));
    }
    __write(buf);
}


function on_data(buf) {
    if (LOG_RAW_DATA) {
        console.log('i', buf.toString('hex'));
    }
    parser.write(buf);
}


if (TEST) {
    //loop back
    __write = (raw) => {
        var b = [];
        for (var i = 0; i < raw.length; i++) {
            b.push(raw[i]); //generate random length of frames (simulation)
            if (Math.random() > 0.7) {
                on_data(new Buffer(b)); //we get this portion
                b = [];
            }
        }
        if (b.length > 0) {
            on_data(new Buffer(b));
        }
    }
    ready();
} else {
    var sp = new serial(SERIAL, {
        baudRate: BAUD,
        autoOpen: false
    });
    sp.open((e) => {
        if (e) {
            console.log("Error openning Serialport");
            console.log(e);
            sp.close();
            return;
        }
        setTimeout(ready, 2000);
        sp.on("data", on_data);
    });
    __write = (raw) => {
        sp.write(raw);
    }

}

function read(buf) {
    try {
        buf = oscmin.fromBuffer(buf, true);
        if (LOG_INCOMING) {
            console.log('Incoming < ');
            console.log(buf);
        }
    } catch (e) {
        console.log("OSC - Malformed OSC Packet");
    }
}

function ready() {
    setTimeout(() => {
        send({
            address: "/init",
            args: []
        });

        setTimeout(() => {
            send({
                address: "/act_simulate_delay",
                args: [{
                    type: "integer",
                    value: 30000
                }]
            });

            function loop() {
                var q = setTimeout(() => {
                    clearTimeout(q);
                    send({
                        address: "/report",
                        args: []
                    });
                    loop();
                }, 50)
            }
            loop();
        }, 5000)
    }, 2000);
}