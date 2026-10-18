// Dumps every function's decompiled C (with entry address) to out/bio4_decomp.c,
// plus a function index (addr, size, name) to out/functions.tsv.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import java.io.*;

public class DumpDecomp extends GhidraScript {
    @Override
    public void run() throws Exception {
        String outDir = getScriptArgs().length > 0 ? getScriptArgs()[0] : "out";
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        try (PrintWriter c = new PrintWriter(new FileWriter(outDir + "/" + getProgramFile().getName() + ".c"));
             PrintWriter idx = new PrintWriter(new FileWriter(outDir + "/" + getProgramFile().getName() + ".tsv"))) {
            FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
            int n = 0;
            while (it.hasNext() && !monitor.isCancelled()) {
                Function f = it.next();
                idx.printf("%s\t%d\t%s%n", f.getEntryPoint(), f.getBody().getNumAddresses(), f.getName());
                DecompileResults r = di.decompileFunction(f, 60, monitor);
                c.printf("//==== %s %s%n", f.getEntryPoint(), f.getName());
                if (r != null && r.decompileCompleted()) c.println(r.getDecompiledFunction().getC());
                else c.println("// decompile failed: " + (r == null ? "null" : r.getErrorMessage()));
                if (++n % 500 == 0) println("decompiled " + n);
            }
            println("done: " + n + " functions");
        }
        di.dispose();
    }
}
