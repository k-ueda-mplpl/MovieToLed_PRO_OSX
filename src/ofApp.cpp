#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	MtL.setup();
	MtL.setupRender(32, 32, ofGetWidth() - 64, ofGetHeight() / 3 - 64, 220);
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
}

//--------------------------------------------------------------
void ofApp::update() {
	MtL.update();
}

//--------------------------------------------------------------
void ofApp::draw() {
	MtL.draw();
}

//--------------------------------------------------------------
void ofApp::exit() {
	MtL.exit();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == OF_KEY_SPACE) {
		MtL.start();
	} else if (key == OF_KEY_SHIFT) {
		MtL.switchWindowSize();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	// MtL.checkAndSetWindowSize(w, h);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
	MtL.dragEvent(dragInfo);
}
