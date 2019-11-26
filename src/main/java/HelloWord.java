import com.BlankPageDetectDLL;

import java.io.IOException;
import java.lang.reflect.Field;

/**
 * @program: YH_JNI
 * @description:
 * @author: Biggee
 * @create: 2019-11-25 11:43
 **/

class HelloWorld {
    static {
        try {
            addDir("jnilibs");
            addDir("src/main/jni/opencv-4.1.2/build/x64/vc15/bin");
            System.loadLibrary("opencv_world412");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    static {
        System.loadLibrary("hello");
    }

    public static void main(String[] args) {
        new HelloWorld().print();
        new HelloWorld01().print();
        try {
            BlankPageDetectDLL.BlankPageDetect("assert/1.jpg", "libs");
        } catch (Exception e) {
            e.printStackTrace();
        }
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
        }
    }

    public native void print();
}