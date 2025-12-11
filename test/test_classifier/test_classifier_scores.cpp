// Classifier Score Normalization Tests
// Tests the score normalization, max finding, and vulnerability scoring
// From: src/ml/inference.cpp

#include <unity.h>
#include "../mocks/testable_functions.h"
#include <cmath>

void setUp(void) {}
void tearDown(void) {}

// ============================================================================
// normalizeScores - Basic Functionality
// ============================================================================

void test_normalize_scores_simple(void) {
    float scores[5] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    
    bool result = normalizeScores(scores, 5);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, scores[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, scores[1]);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, scores[2]);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, scores[3]);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, scores[4]);
}

void test_normalize_scores_sum_to_one(void) {
    float scores[5] = {0.3f, 0.5f, 0.1f, 0.2f, 0.4f};
    
    normalizeScores(scores, 5);
    
    float sum = scores[0] + scores[1] + scores[2] + scores[3] + scores[4];
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, sum);
}

void test_normalize_scores_preserves_proportions(void) {
    float scores[3] = {2.0f, 4.0f, 6.0f};  // 1:2:3 ratio
    
    normalizeScores(scores, 3);
    
    // Should maintain ratio: scores[1] = 2*scores[0], scores[2] = 3*scores[0]
    TEST_ASSERT_FLOAT_WITHIN(0.001f, scores[0] * 2.0f, scores[1]);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, scores[0] * 3.0f, scores[2]);
}

void test_normalize_scores_all_zeros(void) {
    float scores[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    
    bool result = normalizeScores(scores, 5);
    
    TEST_ASSERT_FALSE(result);  // Cannot normalize zeros
    // Scores should remain zero
    TEST_ASSERT_EQUAL_FLOAT(0.0f, scores[0]);
}

void test_normalize_scores_single_element(void) {
    float scores[1] = {5.0f};
    
    bool result = normalizeScores(scores, 1);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, scores[0]);
}

void test_normalize_scores_single_nonzero(void) {
    float scores[5] = {0.0f, 0.0f, 0.8f, 0.0f, 0.0f};
    
    normalizeScores(scores, 5);
    
    TEST_ASSERT_EQUAL_FLOAT(1.0f, scores[2]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, scores[0]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, scores[4]);
}

// ============================================================================
// findMaxIndex - Basic Functionality
// ============================================================================

void test_find_max_index_first(void) {
    float values[5] = {0.9f, 0.1f, 0.2f, 0.3f, 0.4f};
    
    size_t idx = findMaxIndex(values, 5);
    
    TEST_ASSERT_EQUAL(0, idx);
}

void test_find_max_index_last(void) {
    float values[5] = {0.1f, 0.2f, 0.3f, 0.4f, 0.9f};
    
    size_t idx = findMaxIndex(values, 5);
    
    TEST_ASSERT_EQUAL(4, idx);
}

void test_find_max_index_middle(void) {
    float values[5] = {0.1f, 0.2f, 0.9f, 0.3f, 0.4f};
    
    size_t idx = findMaxIndex(values, 5);
    
    TEST_ASSERT_EQUAL(2, idx);
}

void test_find_max_index_equal_values(void) {
    float values[5] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    
    size_t idx = findMaxIndex(values, 5);
    
    // First occurrence wins for equal values
    TEST_ASSERT_EQUAL(0, idx);
}

void test_find_max_index_empty_array(void) {
    float values[1] = {0.0f};
    
    size_t idx = findMaxIndex(values, 0);
    
    TEST_ASSERT_EQUAL(0, idx);  // Default for empty
}

void test_find_max_index_negative_values(void) {
    float values[5] = {-0.5f, -0.1f, -0.9f, -0.3f, -0.2f};
    
    size_t idx = findMaxIndex(values, 5);
    
    TEST_ASSERT_EQUAL(1, idx);  // -0.1 is the maximum
}

// ============================================================================
// clampScore - Edge Cases
// ============================================================================

void test_clamp_score_in_range(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.5f, clampScore(0.5f));
}

void test_clamp_score_zero(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, clampScore(0.0f));
}

void test_clamp_score_one(void) {
    TEST_ASSERT_EQUAL_FLOAT(1.0f, clampScore(1.0f));
}

void test_clamp_score_negative(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, clampScore(-0.5f));
}

void test_clamp_score_over_one(void) {
    TEST_ASSERT_EQUAL_FLOAT(1.0f, clampScore(1.5f));
}

void test_clamp_score_large_negative(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, clampScore(-100.0f));
}

void test_clamp_score_large_positive(void) {
    TEST_ASSERT_EQUAL_FLOAT(1.0f, clampScore(100.0f));
}

// ============================================================================
// calculateVulnScore - Security Feature Combinations
// ============================================================================

void test_vuln_score_open_network(void) {
    float score = calculateVulnScore(false, false, false, false, false);
    
    TEST_ASSERT_EQUAL_FLOAT(0.5f, score);  // Open = 0.5
}

void test_vuln_score_wpa_only(void) {
    float score = calculateVulnScore(true, false, false, false, false);
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.4f, score);  // WPA1 only = 0.4
}

void test_vuln_score_wpa2_secure(void) {
    float score = calculateVulnScore(false, true, false, false, false);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, score);  // WPA2 = secure
}

void test_vuln_score_wpa3_secure(void) {
    float score = calculateVulnScore(false, false, true, false, false);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, score);  // WPA3 = secure
}

void test_vuln_score_wps_enabled(void) {
    float score = calculateVulnScore(false, true, false, true, false);
    
    TEST_ASSERT_EQUAL_FLOAT(0.2f, score);  // WPS on WPA2 = 0.2
}

void test_vuln_score_open_with_wps(void) {
    float score = calculateVulnScore(false, false, false, true, false);
    
    // Open (0.5) + WPS (0.2) = 0.7
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.7f, score);
}

void test_vuln_score_hidden_open(void) {
    float score = calculateVulnScore(false, false, false, false, true);
    
    // Open (0.5) + Hidden with vuln > 0.3 (+0.1) = 0.6
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.6f, score);
}

void test_vuln_score_hidden_wpa2(void) {
    float score = calculateVulnScore(false, true, false, false, true);
    
    // WPA2 = 0.0, hidden bonus doesn't apply (vuln not > 0.3)
    TEST_ASSERT_EQUAL_FLOAT(0.0f, score);
}

void test_vuln_score_wpa_wpa2_mixed(void) {
    float score = calculateVulnScore(true, true, false, false, false);
    
    // WPA+WPA2 = not WPA-only, so no vuln
    TEST_ASSERT_EQUAL_FLOAT(0.0f, score);
}

// ============================================================================
// calculateDeauthScore - Signal and Protection
// ============================================================================

void test_deauth_score_good_signal_no_wpa3(void) {
    float score = calculateDeauthScore(-50, false);
    
    // Good signal (0.2) + no WPA3 (0.3) = 0.5
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, score);
}

void test_deauth_score_weak_signal_no_wpa3(void) {
    float score = calculateDeauthScore(-80, false);
    
    // Weak signal (0.0) + no WPA3 (0.3) = 0.3
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.3f, score);
}

void test_deauth_score_good_signal_wpa3(void) {
    float score = calculateDeauthScore(-50, true);
    
    // Good signal (0.2) + WPA3 (0.0) = 0.2
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, score);
}

void test_deauth_score_suspicious_signal(void) {
    float score = calculateDeauthScore(-20, false);
    
    // Suspiciously strong (0.0) + no WPA3 (0.3) = 0.3
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.3f, score);
}

void test_deauth_score_boundary_low(void) {
    // Exactly at -70 boundary
    float score = calculateDeauthScore(-70, false);
    
    // rssi > -70 is false (not strictly greater), so 0.0 + 0.3 = 0.3
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.3f, score);
}

void test_deauth_score_boundary_high(void) {
    // Exactly at -30 boundary
    float score = calculateDeauthScore(-30, false);
    
    // rssi < -30 is false (not strictly less), so 0.0 + 0.3 = 0.3
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.3f, score);
}

void test_deauth_score_optimal_range(void) {
    // -69 is in optimal range (-70 < rssi < -30)
    float score = calculateDeauthScore(-69, false);
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, score);
}

// ============================================================================
// calculateEvilTwinScore - Hidden + Strong Signal
// ============================================================================

void test_evil_twin_score_hidden_strong(void) {
    float score = calculateEvilTwinScore(true, -40);
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, score);
}

void test_evil_twin_score_hidden_weak(void) {
    float score = calculateEvilTwinScore(true, -70);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, score);
}

void test_evil_twin_score_visible_strong(void) {
    float score = calculateEvilTwinScore(false, -40);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, score);
}

void test_evil_twin_score_visible_weak(void) {
    float score = calculateEvilTwinScore(false, -70);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, score);
}

void test_evil_twin_score_boundary(void) {
    // Exactly -50 is not > -50
    float score = calculateEvilTwinScore(true, -50);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, score);
}

void test_evil_twin_score_just_above_threshold(void) {
    // -49 is > -50
    float score = calculateEvilTwinScore(true, -49);
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, score);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // normalizeScores
    RUN_TEST(test_normalize_scores_simple);
    RUN_TEST(test_normalize_scores_sum_to_one);
    RUN_TEST(test_normalize_scores_preserves_proportions);
    RUN_TEST(test_normalize_scores_all_zeros);
    RUN_TEST(test_normalize_scores_single_element);
    RUN_TEST(test_normalize_scores_single_nonzero);
    
    // findMaxIndex
    RUN_TEST(test_find_max_index_first);
    RUN_TEST(test_find_max_index_last);
    RUN_TEST(test_find_max_index_middle);
    RUN_TEST(test_find_max_index_equal_values);
    RUN_TEST(test_find_max_index_empty_array);
    RUN_TEST(test_find_max_index_negative_values);
    
    // clampScore
    RUN_TEST(test_clamp_score_in_range);
    RUN_TEST(test_clamp_score_zero);
    RUN_TEST(test_clamp_score_one);
    RUN_TEST(test_clamp_score_negative);
    RUN_TEST(test_clamp_score_over_one);
    RUN_TEST(test_clamp_score_large_negative);
    RUN_TEST(test_clamp_score_large_positive);
    
    // calculateVulnScore
    RUN_TEST(test_vuln_score_open_network);
    RUN_TEST(test_vuln_score_wpa_only);
    RUN_TEST(test_vuln_score_wpa2_secure);
    RUN_TEST(test_vuln_score_wpa3_secure);
    RUN_TEST(test_vuln_score_wps_enabled);
    RUN_TEST(test_vuln_score_open_with_wps);
    RUN_TEST(test_vuln_score_hidden_open);
    RUN_TEST(test_vuln_score_hidden_wpa2);
    RUN_TEST(test_vuln_score_wpa_wpa2_mixed);
    
    // calculateDeauthScore
    RUN_TEST(test_deauth_score_good_signal_no_wpa3);
    RUN_TEST(test_deauth_score_weak_signal_no_wpa3);
    RUN_TEST(test_deauth_score_good_signal_wpa3);
    RUN_TEST(test_deauth_score_suspicious_signal);
    RUN_TEST(test_deauth_score_boundary_low);
    RUN_TEST(test_deauth_score_boundary_high);
    RUN_TEST(test_deauth_score_optimal_range);
    
    // calculateEvilTwinScore
    RUN_TEST(test_evil_twin_score_hidden_strong);
    RUN_TEST(test_evil_twin_score_hidden_weak);
    RUN_TEST(test_evil_twin_score_visible_strong);
    RUN_TEST(test_evil_twin_score_visible_weak);
    RUN_TEST(test_evil_twin_score_boundary);
    RUN_TEST(test_evil_twin_score_just_above_threshold);
    
    return UNITY_END();
}
