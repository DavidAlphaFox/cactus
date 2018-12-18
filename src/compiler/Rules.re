type compile_target = {
  input: Fpath.t,
  output: Fpath.t,
};

type compilation_unit = [ | `Create_dir(Fpath.t) | `Compile(compile_target)];

let compile: (Fpath.t, compile_target) => Lwt.t(unit) =
  (output_dir, target) => {
    open Lwt.Infix;
    let final_out_path =
      Fpath.append(output_dir, target.output) |> Fpath.to_string;
    Lwt.catch(
      () =>
        target.input
        |> Fpath.to_string
        |> Base.OS.Async.readfile
        >|= (
          content =>
            content
            |> Omd.of_string
            |> Omd.to_html
            |> Markup.string
            |> Markup.parse_html
            |> Markup.signals
            |> Markup.pretty_print
            |> Markup.write_html
            |> Markup.to_string
        )
        >>= Base.OS.Async.writefile(final_out_path),
      exc =>
        Logs_lwt.err(m =>
          m("Something went wrong! Error: %s", exc |> Printexc.to_string)
        ),
    );
  };

let mkdir = Base.OS.mkdirp;

/* TODO(@ostera): rewrite to use fprintf instead */
let pp = rule =>
  Format.(
    switch (rule) {
    | `Create_dir(name) =>
      print_string("mkdir " ++ name);
      print_newline();
    | `Compile({input, output}) =>
      print_string(
        Fpath.to_string(input) ++ " => " ++ Fpath.to_string(output),
      );
      print_newline();
    }
  );
