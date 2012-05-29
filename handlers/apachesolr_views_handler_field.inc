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

}