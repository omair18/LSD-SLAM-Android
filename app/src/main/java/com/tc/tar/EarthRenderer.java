package com.tc.tar;

import android.content.Context;
import android.view.MotionEvent;

import org.rajawali3d.Object3D;
import org.rajawali3d.lights.DirectionalLight;
import org.rajawali3d.materials.Material;
import org.rajawali3d.materials.methods.DiffuseMethod;
import org.rajawali3d.materials.textures.ATexture;
import org.rajawali3d.materials.textures.Texture;
import org.rajawali3d.math.vector.Vector3;
import org.rajawali3d.primitives.Sphere;
import org.rajawali3d.renderer.Renderer;

/**
 * Created by aarontang on 2017/3/16.
 */

public class EarthRenderer extends Renderer {

    private Object3D mSphere;

    public EarthRenderer(Context context) {
        super(context);
    }

    @Override
    protected void initScene() {
        final DirectionalLight directionalLight = new DirectionalLight();
        directionalLight.setPosition(1.0f, 0.2f, 1.0f);
        directionalLight.setPower(2.0f);
        directionalLight.setLookAt(Vector3.ZERO);
        directionalLight.enableLookAt();
        getCurrentScene().addLight(directionalLight);

        try {
            Material material = new Material();
            material.enableLighting(true);
            material.setDiffuseMethod(new DiffuseMethod.Lambert());
            material.setColor(0);
            material.setColorInfluence(0);
            material.addTexture(new Texture("earthColors",
                    R.drawable.earthtruecolor_nasa_big));
            mSphere = new Sphere(1, 24, 24);
            mSphere.setMaterial(material);
            mSphere.setAlpha(0.2f);
            getCurrentScene().addChild(mSphere);
        } catch (ATexture.TextureException e) {
            e.printStackTrace();
        }

        getCurrentCamera().enableLookAt();
        getCurrentCamera().setLookAt(0, 0, 0);
        getCurrentCamera().setZ(6);
    }

    @Override
    protected void onRender(long ellapsedRealtime, double deltaTime) {
        super.onRender(ellapsedRealtime, deltaTime);
        mSphere.rotate(Vector3.Axis.Y, 1.0);
    }

    @Override
    public void onOffsetsChanged(float xOffset, float yOffset, float xOffsetStep, float yOffsetStep, int xPixelOffset, int yPixelOffset) {

    }

    @Override
    public void onTouchEvent(MotionEvent event) {

    }
}
