/* Read javascript code from stdin and execute it.

 */
let STACKLEN = false;
const exhandlers = [];

function TRY(k, f) {
    const frame = [];
    for (let i = 2; i < arguments.length;) {
        const x = {
            code: arguments[i++],
            handler: arguments[i++],
            continuation: k
        };
        if (typeof x.handler !== 'function') {
            throw new Error('Exception handler must be a function!');
        }
        frame.push(x);
    }
    exhandlers.push(frame);
    f(function (result) {
        exhandlers.pop();
        k(result);
    })
}

function THROW(discarded, code, info) {
    while (exhandlers.length > 0) {
        const frame = exhandlers.pop();
        for (let i = 0; i < frame.length; i++) {
            const x = frame[i];
            if (x.code === true || x.code === code) {
                x.handler(x.continuation, info, code);
                return;
            }
        }
    }
    throw new Error("No error handler for [" + code + '] ' + info);
}

function GUARD(args, f) {
    if (--STACKLEN < 0) throw new Continuation(f, args);
}

function Continuation(f, args) {
    this.f = f;
    this.args = args;
}

function Execute(f, args) {
    // console.log('entering');
    // if (IN_EXECUTE)
    // {
    //     console.log('here');
    //     return f.apply(null, args);
    // }
    // IN_EXECUTE = true;
    while (true)
        try {
            STACKLEN = 200;
            f.apply(null, args);
            break;
        } catch (ex) {
            if (ex instanceof Continuation) {
                f = ex.f;
                args = ex.args;
            } else {
                // IN_EXECUTE = false;
                throw ex;
            }
        }
    // IN_EXECUTE = false;
}

if (typeof process != "undefined") (function () {
    let code = "";
    process.stdin.setEncoding("utf8");
    process.stdin.on("readable", function () {
        const chunk = process.stdin.read();
        if (chunk) code += chunk;
    });
    process.stdin.on("end", function () {
        const func = new Function("TOPLEVEL, GUARD, Execute, require, TRY, THROW", code);
        // console.log(func.toString());
        console.error("/*");
        console.time("Runtime");
        Execute(func, [
            function (result) {
                console.timeEnd("Runtime");
                console.error("***Result: " + result);
                console.error("*/");
            },
            GUARD,
            Execute,
            require,
            TRY,
            THROW
        ]);
        // result = eval(code);
        // console.log("The result is " + result)
    });
})();
