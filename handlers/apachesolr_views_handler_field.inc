<?php

/**
 * Field handler for Solr fields.
 */
class apachesolr_views_handler_field extends views_handler_field {

  // Retrieve value from Solr result document.
  function get_value($values, $field = NULL) {
    $alias = isset($field) ? $this->aliases[$field] : $this->field_alias;
    if (isset($values->{$alias})) {
      return $values->{$alias};
    }
  }

  /**
   * Called to determine what to tell the clicksorter.
   */
  function click_sort($order) {
    $order = strtolower($order);

    // Check if click sort field is available and use it.
    $sort_field = $this->real_field;
    if (isset($this->definition['click sort field'])) {
      $sort_field = $this->definition['click sort field'];
    }

    $this->query->add_sort($sort_field, $order);
  }

}