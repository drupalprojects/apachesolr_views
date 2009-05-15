<?php
// $Id$
class apachesolr_views_handler_argument_mlt extends views_handler_argument_node_nid {
  
  /**
   * Override option_definition() to provide defaults
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['mlt_fl'] = array('default' => array('title' => 'title', 'taxonomy_names' => 'taxonomy_names'));
    $options['mlt_mintf'] = array('default' => '1');
    $options['mlt_mindf'] = array('default' => '1');
    $options['mlt_minwl'] = array('default' => '3');
    $options['mlt_maxwl'] = array('default' => '15');
    $options['mlt_maxqt'] = array('default' => '30');
    return $options;
  }
  /**
   * Override options form to present specific stuffs
   */
  function options_form(&$form, &$form_state) {
    module_load_include('inc', 'apachesolr', 'apachesolr.admin'); // for apachesolr_mlt_get_fields()
    parent::options_form($form, $form_state);
    // TODO: figure out how to do this. cant do id: 123 id: 456 can we?
    unset($form['break_phrase']);
    
    // TODO: can we exclude this argument??
    unset($form['not']);
    
    $form['mlt_fl'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Fields for finding related content'),
      '#description' => t('Choose the fields to be used in calculating similarity. The default combination of %taxonomy_names and %title will provide relevant results for typical sites.', array("%taxonomy_names" => _apachesolr_field_name_map("taxonomy_names"), "%title" => _apachesolr_field_name_map("title"))),
      '#options' => apachesolr_mlt_get_fields(),
      '#required' => TRUE,
      '#default_value' =>  $this->options['mlt_fl'],
    );
   
    $options = drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7));
    $form['mlt_mintf'] = array(
      '#type' => 'select',
      '#title' => t('Minimum Term Frequency'),
      '#description' => t('A word must appear this many times in any given document before the document is considered relevant for comparison.'),
      '#default_value' => $this->options['mlt_mintf'],
      '#options' => $options,
    );
    $form['mlt_mindf'] = array(
      '#type' => 'select',
      '#title' => t('Minimum Document Frequency'),
      '#description' => t('A word must occur in at least this many documents before it will be used for similarity comparison.'),
      '#default_value' => $this->options['mlt_mindf'],
      '#options' => $options,
    );
    $form['mlt_minwl'] = array(
      '#type' => 'select',
      '#title' => t('Minimum Word Length'),
      '#description' => 'You can use this to eliminate short words such as "the" and "it" from similarity comparisons. Words must be at least this number of characters or they will be ignored.',
      '#default_value' => $this->options['mlt_minwl'],
      '#options' => $options,
    );
    $form['mlt_maxwl'] = array(
      '#type' => 'select',
      '#title' => t('Maximum World Length'),
      '#description' => t('You can use this to eliminate very long words from similarity comparisons. Words of more than this number of characters will be ignored.'),
      '#default_value' => $this->options['mlt_maxwl'],
      '#options' => drupal_map_assoc(array(8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20)),
    );
    $form['mlt_maxqt'] = array(
      '#type' => 'select',
      '#title' => t('Maximum number of query terms'),
      '#description' => t('The maximum number of query terms that will be included in any query. Lower numbers will result in fewer recommendations but will get results faster. If a content recommendation is not returning any recommendations, you can either check more "Comparison fields" checkboxes or increase the maximum number of query terms here.'),
      '#options' => drupal_map_assoc(array(5, 10, 15, 20, 25, 30, 35, 40, 45, 50)),
      '#default_value' => $this->options['mlt_maxqt'],
    );
  }
  
  /**
   * Override query().
   */
  function query() {
    $mlt_specific_options = array(
      'mlt_mintf' => 'mlt.mintf',
      'mlt_mindf' => 'mlt.mindf',
      'mlt_minwl' => 'mlt.minwl',
      'mlt_maxwl' => 'mlt.maxwl',
      'mlt_maxqt' => 'mlt.maxqt',
      'mlt_boost' => 'mlt.boost',
      'mlt_qf' => 'mlt.qf',
    );
    $this->query->change_query_template('mlt');
    $this->query->set_param('mlt.fl', implode(',', $this->options['mlt_fl']));
    
    // now set the configuration options for the calculation
    foreach ($mlt_specific_options as $form_key => $option_name) {
      if (!empty($this->options[$form_key])) {
        $this->query->set_param($option_name, $this->options[$form_key]);
      }
    }
    if (!empty($this->options['break_phrase'])) {
      views_break_phrase($this->argument, $this);
    }
    else {
      $this->value = array($this->argument);
    }
    foreach ($this->value as $nid) {
      $this->query->set_query('id:' . apachesolr_document_id($nid));
    }
  }
}
?>