package net.vicp.biggee.jni;

import java.io.IOException;
import java.lang.reflect.Field;

/**
 * @program: YH_JNI
 * @description:
 * @author: Biggee
 * @create: 2019-11-27 08:45
 **/
public class YH_JNI_main {
    public static void initJnilibPath() {
        try {
            addDir("jnilibs");
            if (System.getProperty("os.name").toLowerCase().startsWith("win")) {
                System.loadLibrary("opencv_world412");
            } else if (System.getProperty("os.name").toLowerCase().startsWith("linux")) {

            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        initJnilibPath();
        Common.loadOpenCV();
        System.loadLibrary("BlankPage_Detect");
        System.loadLibrary("IdentityCard");
        System.loadLibrary("RemoveBlackBorder");
        System.loadLibrary("RectifyUnlineDll");
    }

    public static void addDir(String s) throws IOException {
        try {
            Field field = ClassLoader.class.getDeclaredField("usr_paths");
            field.setAccessible(true);
            String[] paths = (String[]) field.get(null);
            for (int i = 0; i < paths.length; i++) {
                if (s.equals(paths[i])) {
                    return;
                }
            }
            String[] tmp = new String[paths.length + 1];
            System.arraycopy(paths, 0, tmp, 0, paths.length);
            tmp[paths.length] = s;
            field.set(null, tmp);
        } catch (IllegalAccessException e) {
            throw new IOException("Failed to get permissions to set library path");
        } catch (NoSuchFieldException e) {
            throw new IOException("Failed to get field handle to set library path");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
