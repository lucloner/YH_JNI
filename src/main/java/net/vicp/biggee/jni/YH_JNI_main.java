package net.vicp.biggee.jni;

import com.BlankPageDetectDLL;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.Field;
import java.nio.charset.StandardCharsets;

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
/*
                final String[] libs = new String[]{
                        "opencv_calib3d",
                        "opencv_contrib",
                        "opencv_core",
                        "opencv_dnn",
                        "opencv_features2d",
                        "opencv_flann",
                        "opencv_gapi",
                        "opencv_gpu",
                        "opencv_highgui",
                        "opencv_imgcodecs",
                        "opencv_imgproc",
                        "opencv_legacy",
                        "opencv_ml",
                        "opencv_objdetect",
                        "opencv_ocl",
                        "opencv_photo",
                        "opencv_stitching",
                        "opencv_superres",
                        "opencv_ts",
                        "opencv_videoio",
                        "opencv_video",
                        "opencv_videostab"
                };
                for (final String lib : libs) {
                    System.loadLibrary(lib);
                }
*/
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        initJnilibPath();
//        File file = new File("assert/test");
//        Long now = System.currentTimeMillis();
//        for (File listFile : file.listFiles()) {
////            RemoveBlackBorderDll.RemoveBlackBorder(""+listFile, "assert/out-"+listFile.getName());
//            System.out.println("***" + listFile + ":" + new BlankPageDetectDLL().BlankPageDetect("" + listFile));
//        }
//        System.out.println(System.currentTimeMillis() - now);

//        System.out.println("---------------------------");
//        IdentityCard.IdentityCardTrans("assert/2.jpg", "assert/out-2.jpg");
        try {
//            System.out.println(new BlankPageDetectDLL().BlankPageDetect("assert/我/0.jpg"));
            System.out.println(new BlankPageDetectDLL().BlankPageDetect(new String("assert/我/0.jpg".getBytes(StandardCharsets.UTF_8), "GBK")));
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
//        RemoveBlackBorderDll.RemoveBlackBorder("assert/4.jpg", "assert/out-4.jpg");
//        JniDemo.ImageRecify("assert/4.jpg", "assert/out-4-1.jpg");  //调整角度
//        JniDemo.RemoveUnline("assert/4.jpg", "assert/out-4-2.jpg", 0, 0, 2366, 2638);
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
