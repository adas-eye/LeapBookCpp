﻿#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Text.h"
#include "cinder/MayaCamUI.h"

#include "Leap.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class LeapApp : public AppNative {
public:

  void setup()
  {
    // ウィンドウの位置とサイズを設定
    setWindowPos( 50, 50 );
    setWindowSize( 1280, 700 );

    // 光源を追加する
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );

    // 表示フォントと領域の設定
    // Mac OSX(Mavericks)とWindows 8.1に標準搭載されているフォント
    mFont = Font( "游ゴシック体", 20 );

    // カメラ(視点)の設定
    float y = 0;
    mCam.setEyePoint( Vec3f( 0.0f, y, 500.0f ) );
    mCam.setCenterOfInterestPoint( Vec3f( 0.0f, y, 0.0f ) );
    mCam.setPerspective( 45.0f, getWindowAspectRatio(), 5.0f, 3000.0f );

    mMayaCam.setCurrentCam(mCam);

    // 描画時に奥行きの考慮を有効にする
    gl::enableDepthRead();

    // Leap Motion関連のセットアップ
    setupLeapObject();
  }

  // マウスダウン
  void mouseDown( MouseEvent event )
  {
    mMayaCam.mouseDown( event.getPos() );
  }

  // マウスのドラッグ
  void mouseDrag( MouseEvent event )
  {
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(),
      event.isMiddleDown(), event.isRightDown() );
  }

  // 更新処理
  void update()
  {
    // フレームの更新
    mLastFrame = mCurrentFrame;
    mCurrentFrame = mLeap.frame();

    updateLeapObject();
    renderFrameParameter();
  }

  // 描画処理
  void draw()
  {
    gl::clear( Color( 0, 0, 0 ) ); 

    drawLeapObject();
    drawTexture();
  }

  // Leap Motion関連のセットアップ
  void setupLeapObject()
  {
    mRotationMatrix = Leap::Matrix::identity();
    mTotalMotionTranslation = Leap::Vector::zero();
    mTotalMotionScale = 1.0f;
  }

  // Leap Motion関連の更新
  void updateLeapObject()
  {
    // 前のフレームからの移動量
    if ( mCurrentFrame.translationProbability( mLastFrame ) > 0.4 ) {
      // 回転を考慮して移動する
      mTotalMotionTranslation +=
        mRotationMatrix
          .rigidInverse()
          .transformDirection( mCurrentFrame.translation( mLastFrame ) );
    }

    // 前のフレームからの回転量
    if ( mCurrentFrame.rotationProbability( mLastFrame ) > 0.4 ) {
      mRotationMatrix *= mCurrentFrame.rotationMatrix( mLastFrame );
    }

    // 前のフレームからの拡大縮小
    if ( mCurrentFrame.scaleProbability( mLastFrame ) > 0.4 ) {
      mTotalMotionScale *= mCurrentFrame.scaleFactor( mLastFrame );
      if ( mTotalMotionScale < 0.1f ) {
        mTotalMotionScale = 0.1f;
      }
    }
  }

  // フレーム情報の描画
  void renderFrameParameter()
  {
    stringstream ss;

    // フレームレート
    ss << "FPS : "<< mCurrentFrame.currentFramesPerSecond() << "\n";

    ss << "Translation :" << mTotalMotionTranslation << "\n";
    ss << "Rotation    :" << mRotationMatrix << "\n";
    ss << "Scale       :" << mTotalMotionScale << "\n";

    // テキストボックスを作成する
    auto tbox = TextBox()
      .alignment( TextBox::LEFT )
      .font( mFont )
      .text ( ss.str() )
      .color(Color( 1.0f, 1.0f, 1.0f ))
      .backgroundColor( ColorA( 0, 0, 0, 0.5f ) );

    mTextTexture = gl::Texture( tbox.render() );
  }

  // Leap Motion関連の描画
  void drawLeapObject()
  {
    // 表示座標系の保持
    gl::pushMatrices();

    // カメラ位置を設定する
    gl::setMatrices( mMayaCam.getCamera() );

    // 表示処理
    glMultMatrixf( mRotationMatrix.toArray4x4() );
    glTranslatef( mTotalMotionTranslation.x,
                  mTotalMotionTranslation.y,
                  mTotalMotionTranslation.z );
    glScalef( mTotalMotionScale, mTotalMotionScale, mTotalMotionScale );
    gl::drawColorCube( Vec3f( 0,0,0 ), Vec3f( 100, 100, 100 ) );

    // 表示座標系を戻す
    gl::popMatrices();
  }

  // テクスチャの描画
  void drawTexture()
  {
    if( mTextTexture ) {
      gl::draw( mTextTexture );
    }
  }

  // GL_LIGHT0の色を変える
  void setDiffuseColor( ci::ColorA diffuseColor )
  {
    gl::color( diffuseColor );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuseColor );
  }

  // Leap SDKのVectorをCinderのVec3fに変換する
  Vec3f toVec3f( Leap::Vector vec )
  {
    return Vec3f( vec.x, vec.y, vec.z );
  }

  // カメラ
  CameraPersp  mCam;
  MayaCamUI    mMayaCam;

  // パラメータ表示用のテクスチャ
  gl::Texture mTextTexture;
  Font mFont;

  // Leap Motion
  Leap::Controller mLeap;
  Leap::Frame mCurrentFrame;
  Leap::Frame mLastFrame;

  Leap::Matrix mRotationMatrix;
  Leap::Vector mTotalMotionTranslation;
  float mTotalMotionScale;
};

CINDER_APP_NATIVE( LeapApp, RendererGl )
