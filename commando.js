const TEST = false;
const SERIAL = "/dev/cu.wchusbserial1410";
const BAUD = 115200;
const LOG_RAW_DATA = false;
const LOG_OUTGOING = true;
const LOG_INCOMING = true;
const LOG_INCOMING_STR = false;

const rl = require('readline');
var _if = rl.createInterface(process.stdin, process.stdout);
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


const Calibration = {
    layer1Z: 12800,
    layer2Z: 24700,
    beltX: 3700,
    beltY: 11000,
    bufferX: 3700,
    bufferY: 30,
    bufferZ: 24700
};


const Board = {
    Z_SENSE_M: 24,
    BELT_EN: 46
};

_if.on('line', (ln) => {
    if (ln == 'report') {
        send({
            address: "/report",
            args: []
        });
    }
    if (ln == 'io') {
        send({
            address: "/readIO",
            args: [{
                type: "integer",
                value: Board.Z_SENSE_M
            },{
                type: "integer",
                value: Board.BELT_EN
            }]
        });
    }
    if (ln == 'init') {
        send({
            address: "/init",
            args: []
        });
    }
    if (ln == 'grab') {
        send({
            address: "/grab",
            args: [{
                type: "integer",
                value: Calibration.layer1Z
            }, {
                type: "integer",
                value: Calibration.layer2Z
            }, {
                type: "integer",
                value: 170 //fromx
            }, {
                type: "integer",
                value: 30 //fromy
            }, {
                type: "integer",
                value: 1 //fromLayer
            }, {
                type: "integer",
                value: Calibration.beltX //belt
            }, {
                type: "integer",
                value: Calibration.beltY //belt
            }, {
                type: "integer",
                value: Calibration.bufferX
            }, {
                type: "integer",
                value: Calibration.bufferY
            }, {
                type: "integer",
                value: Calibration.bufferZ
            }]
        });
    }
    if (ln == 'back') {
        send({
            address: "/retract",
            args: [{
                type: "integer",
                value: Calibration.layer1Z
            }, {
                type: "integer",
                value: Calibration.layer2Z
            }, {
                type: "integer",
                value: 170 //fromx
            }, {
                type: "integer",
                value: 30 //fromy
            }, {
                type: "integer",
                value: 1 //fromLayer
            }, {
                type: "integer",
                value: Calibration.beltX //belt
            }, {
                type: "integer",
                value: Calibration.beltY //belt
            }, {
                type: "integer",
                value: Calibration.bufferX
            }, {
                type: "integer",
                value: Calibration.bufferY
            }, {
                type: "integer",
                value: Calibration.bufferZ
            }]
        });
    }
});
function ready() {
    console.log('ready');
    // setTimeout(() => {

    /**
     * 
struct params {

int layer1Z;
int layer2Z;

int fromX;
int fromY;
int fromZ;

int beltX;
int beltY;

int bufferX;
int bufferY;
int bufferZ;

};

//      */
    //     setTimeout(() => {

    //     }, 3000);
    // }, 2000);
}