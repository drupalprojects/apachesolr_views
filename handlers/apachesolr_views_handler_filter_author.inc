<?php

/* $Id$ */

/**
 * Class for filtering by users.
 */
class apachesolr_views_handler_filter_author extends views_handler_filter {
  
  public function option_definition() {
    $options = parent::option_definition();

    $options['operator'] = array('default' => 'one of');
    $options['value'] = array('default' => '');

    return $options;
  }
  
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $time = time();

    $form['operator'] = array(
      '#type' => 'select',
      '#title' => t('Filter for'),
      '#options' => array(
        'one of' => t('One of these'),
        'not one of' => t('Not one of these'),
        'current' => t('Current user'),
        'not current' => t('Not current user'),
      ),
      '#default_value' =>
          isset($this->options['operator'])
          ? $this->options['operator']
          : 'one of',
    );
    $form['value'] = array(
      '#type' => 'textfield',
      '#title' => t('Usernames'),
      '#description' => t('Enter a comma seperated list of user names.'),
      '#default_value' =>
          isset($this->options['value'])
          ? $this->options['value']
          : '',
      '#process' => array('views_process_dependency'),
      '#dependency' =>
          array('edit-options-operator' => array('one of', 'not one of')),
    );
  }

  public function query() {
    if ($this->options['operator'] == 'current') {
      global $user;
      $this->query->add_term($user->uid, 'uid');
    }
    else if ($this->options['operator'] == 'not current') {
      global $user;
      $this->query->add_term('NOT uid:' . $user->uid);
    }
    else {
      $not = $this->options['operator'] == 'not one of';
      $names = array_map('trim', explode(',', $this->options['value']));
      foreach ($names as $i => $name) {
        if (empty($name)) {
          unset($names[$i]);
        }
      }
      if (empty($names)) {
        if (!$not) {
          $this->query->add_term('nid:-1');//Add term that will yield no results
        }
      }
      else if (count($names) == 1) {
        $this->query->add_term(($not ? 'NOT ' : '') . 'name:' . 
            array_shift($names));
      }
      else {
        $key = 'name:' . array_shift($names);
        foreach ($names as $name) {
          $key .= ' OR name:' . $name;
        }
        $this->query->add_term($not ? 'NOT (' . $key . ')' : $key);
      }
    }
  }
  
}
