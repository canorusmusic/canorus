# This is main entrypoint to pycl2 function (successor to pycli).
# It can't be named pycli2, because we are executing custom logic on any python module that contains "pycli" string.
# TODO(stefan): if pycl2 works, remove pycli and all python threading stuff from canorus.

import CanorusPython
import code, io, sys

console = code.InteractiveConsole()

# main is called every time user enters a text.
def main(document, cmd):
    console.locals['document'] = document
    console.locals['CanorusPython'] = CanorusPython
    oldStdout, oldStderr = sys.stdout, sys.stderr
    newStdout, newStderr = io.StringIO(), io.StringIO()
    sys.stdout, sys.stderr = newStdout, newStderr

    if console.push(cmd):
        # console requires more input.
        prompt = "..."
    else:
        prompt = ">>>"

    sys.stdout = oldStdout
    return "%s%s%s " % (newStdout.getvalue(), newStderr.getvalue(), prompt)
