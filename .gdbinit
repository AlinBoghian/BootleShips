python
import gdb

WATCH_ADDR = 0x10000
OFFSET_ADDR = 0x10008

def handler(event):
    global watch
    if watch.hit_count == 0:
        return
    base = int(gdb.parse_and_eval(f"*(unsigned long long *){OFFSET_ADDR:#x}"))
    print(f"base used {base:#x}")
    gdb.execute("symbol-file")
    gdb.execute(f"add-symbol-file main.efi.debug -o {base:#x}")
    gdb.execute("b efi_main")
    gdb.events.stop.disconnect(handler)
    watch.delete()

gdb.events.stop.connect(handler)
watch = gdb.Breakpoint(f"*(unsigned long long *){WATCH_ADDR:#x} == 0xDEADBEEF",
                       gdb.BP_WATCHPOINT, internal=False)
gdb.execute("continue")

end