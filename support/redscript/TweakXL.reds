
public abstract native class TweakXL {

  public final static native func Reload() -> Void;

  public final static native func ImportAll() -> Void;

  public final static native func ImportDir(path: String) -> Void;

  public final static native func Import(path: String) -> Void;

  public final static native func ExecuteAll() -> Void;

  public final static native func Execute(name: CName) -> Void;
}
