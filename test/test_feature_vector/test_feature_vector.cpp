// Feature Vector Mapping Tests
// Tests the toFeatureVectorRaw function and index mapping
// From: src/ml/features.cpp

#include <unity.h>
#include "../mocks/testable_functions.h"

void setUp(void) {}
void tearDown(void) {}

// ============================================================================
// Feature Index Constants
// ============================================================================

void test_feature_index_rssi_is_0(void) {
    TEST_ASSERT_EQUAL(0, FI_RSSI);
}

void test_feature_index_noise_is_1(void) {
    TEST_ASSERT_EQUAL(1, FI_NOISE);
}

void test_feature_index_snr_is_2(void) {
    TEST_ASSERT_EQUAL(2, FI_SNR);
}

void test_feature_index_channel_is_3(void) {
    TEST_ASSERT_EQUAL(3, FI_CHANNEL);
}

void test_feature_index_has_wps_is_8(void) {
    TEST_ASSERT_EQUAL(8, FI_HAS_WPS);
}

void test_feature_index_has_wpa3_is_11(void) {
    TEST_ASSERT_EQUAL(11, FI_HAS_WPA3);
}

void test_feature_index_is_hidden_is_12(void) {
    TEST_ASSERT_EQUAL(12, FI_IS_HIDDEN);
}

void test_feature_index_anomaly_score_is_22(void) {
    TEST_ASSERT_EQUAL(22, FI_ANOMALY_SCORE);
}

void test_feature_index_padding_start_is_23(void) {
    TEST_ASSERT_EQUAL(23, FI_PADDING_START);
}

void test_feature_index_vector_size_is_32(void) {
    TEST_ASSERT_EQUAL(32, FI_VECTOR_SIZE);
}

// ============================================================================
// toFeatureVectorRaw - Basic Mapping
// ============================================================================

void test_feature_vector_rssi_mapping(void) {
    TestWiFiFeatures f = {0};
    f.rssi = -65;
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(-65.0f, output[FI_RSSI]);
}

void test_feature_vector_noise_mapping(void) {
    TestWiFiFeatures f = {0};
    f.noise = -95;
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(-95.0f, output[FI_NOISE]);
}

void test_feature_vector_snr_mapping(void) {
    TestWiFiFeatures f = {0};
    f.snr = 25.5f;
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(25.5f, output[FI_SNR]);
}

void test_feature_vector_channel_mapping(void) {
    TestWiFiFeatures f = {0};
    f.channel = 6;
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(6.0f, output[FI_CHANNEL]);
}

void test_feature_vector_beacon_interval_mapping(void) {
    TestWiFiFeatures f = {0};
    f.beaconInterval = 100;
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(100.0f, output[FI_BEACON_INTERVAL]);
}

// ============================================================================
// toFeatureVectorRaw - Capability Splitting
// ============================================================================

void test_feature_vector_capability_low_byte(void) {
    TestWiFiFeatures f = {0};
    f.capability = 0x1234;  // Low byte = 0x34
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(0x34, output[FI_CAPABILITY_LO]);
}

void test_feature_vector_capability_high_byte(void) {
    TestWiFiFeatures f = {0};
    f.capability = 0x1234;  // High byte = 0x12
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(0x12, output[FI_CAPABILITY_HI]);
}

void test_feature_vector_capability_zero(void) {
    TestWiFiFeatures f = {0};
    f.capability = 0x0000;
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[FI_CAPABILITY_LO]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[FI_CAPABILITY_HI]);
}

void test_feature_vector_capability_max(void) {
    TestWiFiFeatures f = {0};
    f.capability = 0xFFFF;
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(255.0f, output[FI_CAPABILITY_LO]);
    TEST_ASSERT_EQUAL_FLOAT(255.0f, output[FI_CAPABILITY_HI]);
}

// ============================================================================
// toFeatureVectorRaw - Boolean to Float Conversion
// ============================================================================

void test_feature_vector_bool_false_is_0(void) {
    TestWiFiFeatures f = {0};
    f.hasWPS = false;
    f.hasWPA = false;
    f.hasWPA2 = false;
    f.hasWPA3 = false;
    f.isHidden = false;
    f.respondsToProbe = false;
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[FI_HAS_WPS]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[FI_HAS_WPA]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[FI_HAS_WPA2]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[FI_HAS_WPA3]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[FI_IS_HIDDEN]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[FI_RESPONDS_PROBE]);
}

void test_feature_vector_bool_true_is_1(void) {
    TestWiFiFeatures f = {0};
    f.hasWPS = true;
    f.hasWPA = true;
    f.hasWPA2 = true;
    f.hasWPA3 = true;
    f.isHidden = true;
    f.respondsToProbe = true;
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(1.0f, output[FI_HAS_WPS]);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, output[FI_HAS_WPA]);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, output[FI_HAS_WPA2]);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, output[FI_HAS_WPA3]);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, output[FI_IS_HIDDEN]);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, output[FI_RESPONDS_PROBE]);
}

// ============================================================================
// toFeatureVectorRaw - Padding Verification
// ============================================================================

void test_feature_vector_padding_all_zeros(void) {
    TestWiFiFeatures f = {0};
    f.rssi = -50;  // Non-zero in used area
    f.anomalyScore = 0.5f;
    float output[32] = {0};
    
    // Pre-fill with garbage to ensure padding clears it
    for (int i = 0; i < 32; i++) output[i] = 99.0f;
    
    toFeatureVectorRaw(f, output);
    
    // Check padding area (indices 23-31)
    for (int i = FI_PADDING_START; i < FI_VECTOR_SIZE; i++) {
        TEST_ASSERT_EQUAL_FLOAT(0.0f, output[i]);
    }
}

void test_feature_vector_padding_count(void) {
    // Padding should be 9 elements (indices 23-31 inclusive)
    int paddingCount = FI_VECTOR_SIZE - FI_PADDING_START;
    TEST_ASSERT_EQUAL(9, paddingCount);
}

// ============================================================================
// toFeatureVectorRaw - Complete Feature Set
// ============================================================================

void test_feature_vector_all_fields_populated(void) {
    TestWiFiFeatures f;
    f.rssi = -55;
    f.noise = -90;
    f.snr = 35.0f;
    f.channel = 11;
    f.secondaryChannel = 0;
    f.beaconInterval = 102;
    f.capability = 0x0411;
    f.hasWPS = true;
    f.hasWPA = false;
    f.hasWPA2 = true;
    f.hasWPA3 = false;
    f.isHidden = false;
    f.responseTime = 25;
    f.beaconCount = 100;
    f.beaconJitter = 2.5f;
    f.respondsToProbe = true;
    f.probeResponseTime = 15;
    f.vendorIECount = 5;
    f.supportedRates = 8;
    f.htCapabilities = 0x6F;
    f.vhtCapabilities = 0x00;
    f.anomalyScore = 0.15f;
    
    float output[32] = {0};
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(-55.0f, output[0]);
    TEST_ASSERT_EQUAL_FLOAT(-90.0f, output[1]);
    TEST_ASSERT_EQUAL_FLOAT(35.0f, output[2]);
    TEST_ASSERT_EQUAL_FLOAT(11.0f, output[3]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[4]);
    TEST_ASSERT_EQUAL_FLOAT(102.0f, output[5]);
    TEST_ASSERT_EQUAL_FLOAT(0x11, output[6]);  // Low byte of 0x0411
    TEST_ASSERT_EQUAL_FLOAT(0x04, output[7]);  // High byte of 0x0411
    TEST_ASSERT_EQUAL_FLOAT(1.0f, output[8]);  // hasWPS
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[9]);  // hasWPA
    TEST_ASSERT_EQUAL_FLOAT(1.0f, output[10]); // hasWPA2
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[11]); // hasWPA3
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[12]); // isHidden
    TEST_ASSERT_EQUAL_FLOAT(25.0f, output[13]);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, output[14]);
    TEST_ASSERT_EQUAL_FLOAT(2.5f, output[15]);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, output[16]); // respondsToProbe
    TEST_ASSERT_EQUAL_FLOAT(15.0f, output[17]);
    TEST_ASSERT_EQUAL_FLOAT(5.0f, output[18]);
    TEST_ASSERT_EQUAL_FLOAT(8.0f, output[19]);
    TEST_ASSERT_EQUAL_FLOAT(0x6F, output[20]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output[21]);
    TEST_ASSERT_EQUAL_FLOAT(0.15f, output[22]);
}

void test_feature_vector_extreme_rssi_positive(void) {
    // Suspiciously strong signal
    TestWiFiFeatures f = {0};
    f.rssi = -10;
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(-10.0f, output[FI_RSSI]);
}

void test_feature_vector_extreme_rssi_negative(void) {
    // Very weak signal
    TestWiFiFeatures f = {0};
    f.rssi = -100;
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(-100.0f, output[FI_RSSI]);
}

void test_feature_vector_high_beacon_count(void) {
    TestWiFiFeatures f = {0};
    f.beaconCount = 65535;  // Max uint16_t
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(65535.0f, output[FI_BEACON_COUNT]);
}

void test_feature_vector_high_response_time(void) {
    TestWiFiFeatures f = {0};
    f.responseTime = 1000000;  // 1 second in microseconds
    float output[32] = {0};
    
    toFeatureVectorRaw(f, output);
    
    TEST_ASSERT_EQUAL_FLOAT(1000000.0f, output[FI_RESPONSE_TIME]);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Feature index constants
    RUN_TEST(test_feature_index_rssi_is_0);
    RUN_TEST(test_feature_index_noise_is_1);
    RUN_TEST(test_feature_index_snr_is_2);
    RUN_TEST(test_feature_index_channel_is_3);
    RUN_TEST(test_feature_index_has_wps_is_8);
    RUN_TEST(test_feature_index_has_wpa3_is_11);
    RUN_TEST(test_feature_index_is_hidden_is_12);
    RUN_TEST(test_feature_index_anomaly_score_is_22);
    RUN_TEST(test_feature_index_padding_start_is_23);
    RUN_TEST(test_feature_index_vector_size_is_32);
    
    // Basic mapping
    RUN_TEST(test_feature_vector_rssi_mapping);
    RUN_TEST(test_feature_vector_noise_mapping);
    RUN_TEST(test_feature_vector_snr_mapping);
    RUN_TEST(test_feature_vector_channel_mapping);
    RUN_TEST(test_feature_vector_beacon_interval_mapping);
    
    // Capability splitting
    RUN_TEST(test_feature_vector_capability_low_byte);
    RUN_TEST(test_feature_vector_capability_high_byte);
    RUN_TEST(test_feature_vector_capability_zero);
    RUN_TEST(test_feature_vector_capability_max);
    
    // Boolean conversion
    RUN_TEST(test_feature_vector_bool_false_is_0);
    RUN_TEST(test_feature_vector_bool_true_is_1);
    
    // Padding
    RUN_TEST(test_feature_vector_padding_all_zeros);
    RUN_TEST(test_feature_vector_padding_count);
    
    // Complete feature set
    RUN_TEST(test_feature_vector_all_fields_populated);
    RUN_TEST(test_feature_vector_extreme_rssi_positive);
    RUN_TEST(test_feature_vector_extreme_rssi_negative);
    RUN_TEST(test_feature_vector_high_beacon_count);
    RUN_TEST(test_feature_vector_high_response_time);
    
    return UNITY_END();
}
